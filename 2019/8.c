#include <aoclib.h>
#include <stdio.h>
#include <limits.h>

#define nrows 6
#define npixels 25

#define INDEX(layer,row,pixel) (layer*nrows*npixels + row*npixels + pixel)

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

        for (size_t layer=0; layer<nlayers; layer++) {
                for (size_t row=0; row<nrows; row++) {
                        for (size_t pixel=0; pixel<npixels; pixel++) {
                                size_t i = INDEX(0, row, pixel);
                                size_t j = INDEX(layer, row, pixel);

                                if (layer == 0 || image[i] == '2') {
                                        image[i] = input[j];
                                }

                                if (layer == nlayers-1) {
                                        switch(image[i]) {
                                        case '0':
                                                fprintf(stderr, " ");
                                                break;
                                        case '1':
                                                fprintf(stderr, "*");
                                                break;
                                        case '2':
                                                fprintf(stderr, "?");
                                                break;
                                        }
                                }
                        }
                        if (layer == nlayers-1) {
                                fprintf(stderr, "\n");
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "See above.");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
