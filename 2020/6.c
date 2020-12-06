#include <aoclib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define NQUESTIONS ('z'-'a'+1)
typedef uint32_t questions;

static int count_set_bits(questions x) {
        int count = 0;
        for (int i=0; i<NQUESTIONS; i++) {
                if (x & 1) {
                        count++;
                }
                x >>= 1;
        }
        return count;
}

static int solution(const char *const input, bool all) {
        int count = 0;
        
        questions person = 0;
        questions group = 0;
        if (all) {
                group = ~group;
        }
        
        for (size_t i=0; input[i]!='\0'; i++) {
                if (input[i] == '\n') {
                        if (all) {
                                group &= person;
                        } else {
                                group |= person;
                        }
                        person = 0;
                        
                        if (input[i+1] == '\n' || input[i+1] == '\0') {
                                count += count_set_bits(group);
                                group = 0;
                                if (all) {
                                        group = ~group;
                                }
                                i++;
                                if (input[i] == '\0') {
                                        break;
                                }
                        }
                } else {
                        int idx = input[i] - 'a';
                        person |= (1<<idx);
                }
        }

        return count;
}

static void solution1(const char *const input, char *const output) {
        int count = solution(input, false);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static void solution2(const char *const input, char *const output) {
        int count = solution(input, true);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
