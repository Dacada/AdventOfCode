#include <aoclib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>

// https://en.wikipedia.org/wiki/Josephus_problem
static void solution1(const char *const input, char *const output) {
        // parse number
        unsigned n = 0;
        for (int i=0; isdigit(input[i]); i++) {
                n *= 10;
                n += input[i] - '0';
        }

        // GCC/Clang only
        int MSB = sizeof(n)*CHAR_BIT - __builtin_clz(n) - 1;

        unsigned mask = ~(1 << MSB);
        unsigned result = ((n & mask) << 1) + 1;
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

// This also has a O(1) solution, make a naive implementation then look for
// patterns.
static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
