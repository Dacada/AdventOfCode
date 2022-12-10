#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int find_cycle(int cycle, int *cycles, int len) {
        if (len <= 0) {
                FAIL("logic error");
        }
        if (len == 1) {
                return 0;
        }

        int *cycles1 = cycles;
        int *cycles2 = cycles + len/2;
        int len1 = len/2;
        int len2 = len/2 + len%2;

        int x = cycles1[len1-1];
        int y = cycles2[0];
        if (cycle == x || (cycle > x && cycle < y)) {
                return len1 - 1;
        } else if (cycle == y) {
                return len1;
        } else if (cycle < x) {
                return find_cycle(cycle, cycles1, len1);
        } else {
                return len1 + find_cycle(cycle, cycles2, len2);
        }
}

static int signal_value(int *cycles, int *values, int len, int cycle) {
        int i = find_cycle(cycle, cycles, len);
        return values[i];
}

static int parse_int(const char **input) {
        bool neg = **input == '-';
        if (neg) {
                *input += 1;
        }
        ASSERT(isdigit(**input), "parse error %c", **input);
        
        int x = 0;
        while (isdigit(**input)) {
                x *= 10;
                x += **input - '0';
                *input += 1;
        }

        if (neg) {
                x = -x;
        }
        
        return x;
}

static inline void skip_newlines(const char **input) {
        while (**input == '\n') {
                *input += 1;
        }
}

#define ASSERT_STR(input, str)                                          \
        ASSERT(strncmp(input, str, sizeof(str)-1) == 0, "parse error"); \
        input += sizeof(str) - 1;

static int parse_input(const char *input, int **cycles, int **values) {
        int len = 0;
        int cap = 16;
        *cycles = malloc(sizeof(**cycles) * cap);
        *values = malloc(sizeof(**values) * cap);

        int cycle = 1;
        int value = 1;
        while (*input != '\0') {
                if (len >= cap) {
                        cap *= 2;
                        *cycles = realloc(*cycles, sizeof(**cycles) * cap);
                        *values = realloc(*values, sizeof(**values) * cap);
                }
                if (*input == 'n') {
                        ASSERT_STR(input, "noop");
                        skip_newlines(&input);
                        cycle++;
                } else if (*input == 'a') {
                        ASSERT_STR(input, "addx ");
                        int x = parse_int(&input);
                        skip_newlines(&input);
                        cycle += 2;
                        value += x;
                        (*cycles)[len] = cycle;
                        (*values)[len] = value;
                        len++;
                } else {
                        FAIL("parse error");
                }
        }

        return len;
}

#undef ASSERT_STR

static void solution1(const char *const input, char *const output) {
        int *cycles;
        int *values;
        int len = parse_input(input, &cycles, &values);

        int res = 0; 
        res += signal_value(cycles, values, len, 20) * 20;
        res += signal_value(cycles, values, len, 60) * 60;
        res += signal_value(cycles, values, len, 100) * 100;
        res += signal_value(cycles, values, len, 140) * 140;
        res += signal_value(cycles, values, len, 180) * 180;
        res += signal_value(cycles, values, len, 220) * 220;
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
        free(cycles);
        free(values);
}

static void solution2(const char *const input, char *const output) {
        int *cycles;
        int *values;
        parse_input(input, &cycles, &values);

        int k = 0;
        int value = 1;
        for (int j=0; j<6; j++) {
                for (int i=0; i<40; i++) {
                        int cycle = j*40 + i + 1;
                        if (cycle == cycles[k]) {
                                value = values[k];
                                k += 1;
                        }

                        char c;
                        if (i >= value - 1 && i <= value + 1) {
                                c = '#';
                        } else {
                                c = ' ';
                        }
                        fputc(c, stderr);
                }
                fputc('\n', stderr);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "SEE STDERR OUTPUT");
        free(cycles);
        free(values);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
