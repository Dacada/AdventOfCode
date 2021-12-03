#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

static unsigned parse_number(const char **const input) {
        unsigned n = 0;
        ASSERT(isdigit(**input), "parse error");
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }
        
        ASSERT(**input == '\n' || **input == '\0', "parse error");
        while (**input == '\n') {
                *input += 1;
        }

        return n;
}

static unsigned *parse_input(const char *input, size_t *const count) {
        *count = 0;
        size_t size = 16;
        unsigned *measurements = malloc(size * sizeof(*measurements));

        while (*input != '\0') {
                if (*count >= size) {
                        size *= 2;
                        measurements = realloc(measurements, size * sizeof(*measurements));
                }
                measurements[*count] = parse_number(&input);
                *count += 1;
        }

        return measurements;
}

static void solution1(const char *const input, char *const output) {
        size_t nmeasurements=0;
        unsigned *measurements = parse_input(input, &nmeasurements);

        unsigned count = 0;
        for (size_t i=1; i<nmeasurements; i++) {
                if (measurements[i] > measurements[i-1]) {
                        count++;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
        free(measurements);
}

static void solution2(const char *const input, char *const output) {
        size_t nmeasurements=0;
        unsigned *measurements = parse_input(input, &nmeasurements);

        unsigned count = 0;
        unsigned cur_window = measurements[0] + measurements[1] + measurements[2];
        for (size_t i=3; i<nmeasurements; i++) {
                unsigned new_window = cur_window - measurements[i-3] + measurements[i];
                if (new_window > cur_window) {
                        count++;
                }
                cur_window = new_window;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
        free(measurements);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
