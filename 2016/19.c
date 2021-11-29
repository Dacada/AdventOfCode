#include <aoclib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

static unsigned parse_number(const char *const input) {
        unsigned n = 0;
        for (int i=0; isdigit(input[i]); i++) {
                n *= 10;
                n += input[i] - '0';
        }
        return n;
}

// https://en.wikipedia.org/wiki/Josephus_problem
static void solution1(const char *const input, char *const output) {
        // parse number
        unsigned n = parse_number(input);

        // GCC/Clang only
        int MSB = sizeof(n)*CHAR_BIT - __builtin_clz(n) - 1;

        unsigned mask = ~(1 << MSB);
        unsigned result = ((n & mask) << 1) + 1;
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

// Pattern is very easily found with a naive solution and small numbers
static void solution2(const char *const input, char *const output) {
        unsigned n = parse_number(input);
        
        unsigned l = pow(3, floor(log(n)/log(3)));
        unsigned result;
        if (l == n) {
                result = n;
        } else if (n <= l*2) {
                result = n - l;
        } else {
                result = (n - l*2)*2 + l;
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
