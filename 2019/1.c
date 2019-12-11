#include <aoclib.h>
#include <stdio.h>

static int parse_int(const char *const input, size_t *const i) {
        size_t j;
        int num = 0;
        for (j=0; j<10; j++) {
                char c = input[*i+j];
                if (c < '0' || c > '9') {
                        break;
                }
                num = num*10 + (c-'0');
        }
        *i += j;
        return num;
}

static int calculate_fuel(int mass) {
        return mass / 3 - 2;
}

static void solution1(const char *const input, char *const output) {
        int sum = 0;
        for (size_t i=0;; i++) {
                if (input[i] == '\0') {
                        break;
                }
                int n = parse_int(input, &i);
                sum += calculate_fuel(n);
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", sum);
}

static void solution2(const char *const input, char *const output) {
        int sum = 0;
        for (size_t i=0;; i++) {
                if (input[i] == '\0') {
                        break;
                }
                int n = parse_int(input, &i);
                int fuel = calculate_fuel(n);
                while (fuel > 0) {
                        sum += fuel;
                        fuel = calculate_fuel(fuel);
                }
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", sum);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
