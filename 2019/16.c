#include <aoclib.h>
#include <stdio.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

static const int pattern[4] = { 0, 1, 0, -1 };

static int *parse(const char *const input, size_t *const ptr_size) {
        size_t size = 64;
        size_t arr_i = 0;
        int *arr = malloc(sizeof(*arr) * size);
        if (arr == NULL) {
                perror("malloc");
                return NULL;
        }

        for (size_t i=0;; i++) {
                char c = input[i];
                if (c > '9' || c < '0') {
                        break;
                }

                while (arr_i >= size) {
                        size *= 2;
                        int *newarr = realloc(arr, sizeof(*newarr) * size);
                        if (newarr == NULL) {
                                perror("realloc");
                                free(arr);
                                return NULL;
                        } else {
                                arr = newarr;
                        }
                }

                arr[arr_i] = c - '0';
                arr_i++;
        }

        int *res = realloc(arr, sizeof(*res) * arr_i);
        if (res == NULL) {
                perror("realloc");
                free(arr);
                return NULL;
        }

        *ptr_size = arr_i;
        return res;
}

static void solution1(const char *const input, char *const output) {
        size_t size;
        int *numbers1 = parse(input, &size);
        if (numbers1 == NULL) {
                snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
                return;
        }
        int *numbers2 = malloc(sizeof(*numbers2) * size);
        if (numbers2 == NULL) {
                perror("malloc");
                snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
                return;
        }

        // Iterate every phase
        for (size_t phase=0; phase<100; phase++) {
                // Iterate every output element
                for (size_t nth=1; nth<=size; nth++) {
                        // Iterate every input element
                        int sum = 0;
                        for (size_t i=0; i<size; i++) {
                                int a = numbers1[i];
                                int b = pattern[((i+1)/nth)%4];
                                sum += a * b;
                        }
                        numbers2[nth-1] = ABS(sum) % 10;
                }

                int *tmp = numbers1;
                numbers1 = numbers2;
                numbers2 = tmp;
        }

        for (int i=0; i<8; i++) {
                char c = numbers1[i] + '0';
                output[i] = c;
        }
        output[8] = '\0';
	free(numbers1);
	free(numbers2);
}

static void solution2(const char *const input, char *const output) {
        size_t size;
        int *numbers = parse(input, &size);
        if (numbers == NULL) {
                return;
        }

        size_t offset = 0;
        for (int i=0; i<7; i++) {
                offset *= 10;
                offset += numbers[i];
        }

        size_t ressize = 10000 * size - offset;
        int *res = malloc(sizeof(*res) * ressize);
        if (res == NULL) {
                perror("malloc");
                snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
                return;
        }

        for (size_t i=0; i<ressize; i++) {
                res[i] = numbers[(offset + i) % size];
        }

        for (size_t phase=0; phase<100; phase++) {
                for (long i = ressize - 2; i >= 0; --i) {
                        res[i] = (res[i] + res[i + 1]) % 10;
                }
        }

        for (int i=0; i<8; i++) {
                char c = res[i] + '0';
                output[i] = c;
        }
        output[8] = '\0';

	free(numbers);
	free(res);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
