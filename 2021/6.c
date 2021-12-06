#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define MAXTIMER 8
#define RESETTIMER 6

static unsigned parse_number(const char **const input) {
        unsigned n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }
        return n;
}

static unsigned *parse_input(const char *input, size_t *const len) {
        size_t size = 16;
        *len = 0;
        unsigned *list = malloc(size * sizeof(*list));

        while (*input != '\0') {
                if (*len >= size) {
                        size *= 2;
                        list = realloc(list, size * sizeof(*list));
                }
                list[*len] = parse_number(&input);
                *len += 1;

                while (*input == '\n') {
                        input += 1;
                }
                ASSERT(*input == ',' || *input == '\0', "parse error");
                if (*input == ',') {
                        input += 1;
                }
        }

        return list;
}

// How many new spawns will a fish at timer=0 be ultimately responsible for
// after the given number of days. There is a better way to implement this
// funciton...
static unsigned long total_spawned_fish(const unsigned ddays, long **const hhistory, const unsigned maxdays) {
        static bool first = true;

        if (first) {
                *hhistory = malloc(sizeof(**hhistory) * (maxdays+1));
        }
        long *history = *hhistory;
        if (first) {
                for (unsigned i=0; i<=maxdays; i++) {
                        history[i] = -1;
                }
                first = false;
        }

        if (history[ddays] > 0) {
                return history[ddays];
        }
        
        unsigned long count = 0;
        unsigned days = ddays;

        if (days != 0) {
                days -= 1;
                count += 1;
                if (days >= MAXTIMER) {
                        count += total_spawned_fish(days - MAXTIMER, hhistory, maxdays);
                }
                
                for (;;) {
                        if (days >= RESETTIMER + 1) {
                                days -= RESETTIMER + 1;
                        } else {
                                break;
                        }
                        count += 1;
                        if (days >= MAXTIMER) {
                                count += total_spawned_fish(days - MAXTIMER, hhistory, maxdays);
                        }
                }
        }

        history[ddays] = count;
        return count;
}

static void solution(const char *const input, char *const output, unsigned days) {
        size_t nages;
        unsigned *timers = parse_input(input, &nages);

        unsigned long count = 0;
        long *history = NULL;
        for (size_t i=0; i<nages; i++) {
                count++;
                count += total_spawned_fish(days - timers[i], &history, days);
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", count);
        free(timers);
        free(history);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 80);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 256);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
