#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define IDX2D(array_, x_, y_, width_) ((array_)[(y_)*(width_)+(x_)])

static unsigned *parse_input(const char *input, size_t *const width, size_t *const height) {
        size_t size = 64;
        size_t i = 0;

        unsigned *nums = malloc(size * sizeof(*nums));
        
        while (*input != '\n') {
                if (i >= size) {
                        size *= 2;
                        nums = realloc(nums, size * sizeof(*nums));
                }
                ASSERT(isdigit(*input), "parse error");
                nums[i++] = *input - '0';
                input++;
        }
        *width = i;
        input++;

        while (*input != '\0') {
                if (i >= size) {
                        size *= 2;
                        nums = realloc(nums, size * sizeof(*nums));
                }
                ASSERT(isdigit(*input), "parse error");
                nums[i++] = *input - '0';
                do {
                        input++;
                } while (*input == '\n');
        }
        *height = i / *width;

        return nums;
}

static void solution1(const char *const input, char *const output) {
        size_t width, height;
        unsigned *nums = parse_input(input, &width, &height);

        unsigned result = 0;
        for (size_t j=0; j<height; j++) {
                for (size_t i=0; i<width; i++) {
                        unsigned n = IDX2D(nums, i, j, width);
                        if ((i == 0 || IDX2D(nums, i-1, j, width) > n) &&
                            (j == 0 || IDX2D(nums, i, j-1, width) > n) &&
                            (i == width-1 || IDX2D(nums, i+1, j, width) > n) &&
                            (j == height-1 || IDX2D(nums, i, j+1, width) > n)) {
                                DBG("%lu,%lu: %u", i, j, n);
                                result += n + 1;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
        free(nums);
}

static void mark_basin(unsigned *nums, unsigned *basins, size_t i, size_t j, size_t width, size_t height, unsigned basin) {
        IDX2D(basins, i, j, width) = basin;
        if (i > 0 && IDX2D(nums, i-1, j, width) != 9 && IDX2D(basins, i-1, j, width) == 0) {
                mark_basin(nums, basins, i-1, j, width, height, basin);
        }
        if (j > 0 && IDX2D(nums, i, j-1, width) != 9 && IDX2D(basins, i, j-1, width) == 0) {
                mark_basin(nums, basins, i, j-1, width, height, basin);
        }
        if (i < width-1 && IDX2D(nums, i+1, j, width) != 9 && IDX2D(basins, i+1, j, width) == 0) {
                mark_basin(nums, basins, i+1, j, width, height, basin);
        }
        if (j < height-1 && IDX2D(nums, i, j+1, width) != 9 && IDX2D(basins, i, j+1, width) == 0) {
                mark_basin(nums, basins, i, j+1, width, height, basin);
        }
}

static int cmp(const void *a, const void *b) {
        const unsigned *n1 = a;
        const unsigned *n2 = b;

        if (*n1 < *n2) {
                return 1;
        } else if (*n1 > *n2) {
                return -1;
        } else {
                return 0;
        }
}

static void solution2(const char *const input, char *const output) {
        size_t width, height;
        unsigned *nums = parse_input(input, &width, &height);
        
        unsigned *basins = malloc(width*height*sizeof(*basins));
        memset(basins, 0, width*height*sizeof(*basins));

        unsigned current_basin = 1;
        for (size_t j=0; j<height; j++) {
                for (size_t i=0; i<width; i++) {
                        if (IDX2D(basins, i, j, width) == 0 && IDX2D(nums, i, j, width) != 9) {
                                mark_basin(nums, basins, i, j, width, height, current_basin);
                                current_basin++;
                        }
                }
        }

        unsigned *basin_sizes = malloc(current_basin * sizeof(*basin_sizes));
        memset(basin_sizes, 0, current_basin * sizeof(*basin_sizes));
        
        for (size_t j=0; j<height; j++) {
                for (size_t i=0; i<width; i++) {
                        unsigned basin = IDX2D(basins, i, j, width);
                        basin_sizes[basin]++;
                }
        }
        basin_sizes[0] = 0;

        qsort(basin_sizes, current_basin-1, sizeof(*basin_sizes), cmp);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", basin_sizes[0] * basin_sizes[1] * basin_sizes[2]);
        free(nums);
        free(basins);
        free(basin_sizes);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
