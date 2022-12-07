#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static inline bool all_different(const char *const buffer, int size) {
        static bool seen[26];
        memset(seen, 0, sizeof(seen));
        for (int i=0; i<size; i++) {
                int j = buffer[i] - 'a';
                if (seen[j]) {
                        return false;
                }
                seen[j] = true;
        }
        return true;
}

static void solution(const char *const input, char *const output, int size) {
        char buffer[size];
        int i;
        for (i=0;; i++) {
                buffer[i % size] = input[i];
                if (i >= size && all_different(buffer, size)) {
                        break;
                }
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", i+1);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 4);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 14);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
