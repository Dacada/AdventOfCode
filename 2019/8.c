#include <aoclib.h>
#include <stdio.h>
#include <limits.h>

#define nrows 6
#define npixels 25

#define INDEX(layer,row,pixel) (layer*nrows*npixels + row*npixels + pixel)

__attribute__((pure))
static size_t parse_image(const char *const input) {
        size_t layer;
        for (layer=0;; layer++) {
                for (size_t row=0; row<nrows; row++) {
                        for (size_t pixel=0; pixel<npixels; pixel++) {
                                size_t i = INDEX(layer, row, pixel);
                                if (input[i] == '\0')
                                        goto end;
                        }
                }
        }

end:
        return layer;
}

static void solution1(const char *const input, char *const output) {
        size_t nlayers = parse_image(input);

        int least_zeros = INT_MAX;
        size_t layer_least_zeros = 0;
        for (size_t layer=0; layer<nlayers; layer++) {
                int zeros = 0;
                for (size_t row=0; row<nrows; row++) {
                        for (size_t pixel=0; pixel<npixels; pixel++) {
                                if (input[INDEX(layer,row,pixel)] == '0') {
                                        zeros++;
                                }
                        }
                }
                
                if (zeros < least_zeros) {
                        //DBG("Layer %lu/%lu has %d zeros", layer, nlayers, zeros);
                        least_zeros = zeros;
                        layer_least_zeros = layer;
                }
        }

        int ones=0, twos=0;
        for (size_t row=0; row<nrows; row++) {
                for (size_t pixel=0; pixel<npixels; pixel++) {
                        int p = input[INDEX(layer_least_zeros, row, pixel)];
                        if (p == '1') {
                                ones++;
                        } else if (p == '2') {
                                twos++;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", ones*twos);
}

static void solution2(const char *const input, char *const output) {
        size_t nlayers = parse_image(input);

        char image[nrows*npixels];

	size_t buffer_cap = 50;
	char *buffer = malloc(sizeof(*buffer)*buffer_cap);
	size_t buffer_len = 0;
	size_t buffer_width = 0;
	size_t buffer_height = 0;
	
        for (size_t layer=0; layer<nlayers; layer++) {
                for (size_t row=0; row<nrows; row++) {
                        for (size_t pixel=0; pixel<npixels; pixel++) {
                                size_t i = INDEX(0, row, pixel);
                                size_t j = INDEX(layer, row, pixel);

                                if (layer == 0 || image[i] == '2') {
                                        image[i] = input[j];
                                }

                                if (layer == nlayers-1) {
				  if (buffer_len >= buffer_cap) {
				    buffer_cap *= 2;
				    buffer = realloc(buffer, sizeof(*buffer)*buffer_cap);
				  }
                                        switch(image[i]) {
                                        case '0':
					  buffer[buffer_len++] = ' ';
                                                break;
                                        case '1':
					  buffer[buffer_len++] = '#';
                                                break;
                                        case '2':
					  buffer[buffer_len++] = '?';
                                                break;
                                        }
                                }
                        }
                        if (layer == nlayers-1) {
			  if (buffer_height == 0) {
			    buffer_width = buffer_len;
			  }
			  buffer_height++;
                        }
                }
        }

	#ifdef DEBUG
	for (size_t j=0; j<buffer_height; j++) {
	  for (size_t i=0; i<buffer_width; i++) {
	    fputc(buffer[j*buffer_width+i], stderr);
	  }
	  fputc('\n', stderr);
	}
	#endif

	char *result = aoc_ocr(buffer, buffer_width, buffer_height);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", result);
	free(buffer);
	free(result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
