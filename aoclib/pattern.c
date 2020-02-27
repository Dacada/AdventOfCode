#include <aoclib.h>
#include <stdio.h>

static void solution1(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
