#include <aoclib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

#define IPFORMAT "%u"
#define IPLIMIT 4294967295
typedef uint32_t ip;

struct range {
        ip min;
        ip max;
};

static int cmp_range(const void *const a, const void *const b) {
        const struct range *aa = a;
        const struct range *bb = b;

        if (aa->min < bb->min) {
                return -1;
        } else if (aa->min > bb->min) {
                return 1;
        } else {
                return 0;
        }
}

static ip parse_number(const char **const input) {
        ip n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                (*input)++;
        }
        return n;
}

static struct range parse_line(const char **const input) {
        ip min = parse_number(input);
        ASSERT(**input == '-', "parse error");
        (*input)++;
        ip max = parse_number(input);
        
        ASSERT(**input == '\n' || **input == '\0', "parse error");
        if (**input == '\n') {
                (*input)++;
        }

        return (struct range){.min = min, .max = max};
}

static struct range *parse_input(const char *input, size_t *const nranges) {
        size_t size = 64;
        size_t current = 0;
        struct range *ranges = malloc(size * sizeof(*ranges));

        while (*input != '\0') {
                if (current >= size) {
                        size *= 2;
                        ranges = realloc(ranges, size * sizeof(*ranges));
                }
                ranges[current] = parse_line(&input);
                current++;
        }

        *nranges = current;
        qsort(ranges, current, sizeof(*ranges), cmp_range);
        return ranges;
}

static void solution1(const char *const input, char *const output) {
        size_t nranges;
        struct range *ranges = parse_input(input, &nranges);

        ip current = ranges[0].min;
        if (current == 0) {
                current = ranges[0].max + 1;
                size_t i;
                for (i=1; i<nranges; i++) {
                        if (ranges[i].min > current) {
                                break;
                        }
                        if (ranges[i].max > current) {
                                current = ranges[i].max + 1;
                        }
                }
                if (i == nranges - 1) {
                        current = ranges[i].max + 1;
                }
        } else {
                current -= 1;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, IPFORMAT "\n", current);
        free(ranges);
}

static void solution2(const char *const input, char *const output) {
        size_t nranges;
        struct range *ranges = parse_input(input, &nranges);

        ip current = 0;
        size_t result = 0;
        for (size_t i=0; i<nranges; i++) {
                if (ranges[i].min > current) {
                        result += ranges[i].min - current;
                }
                
                if (ranges[i].max == IPLIMIT) {
                        current = IPLIMIT;
                        break;
                } else if (ranges[i].max > current) {
                        current = ranges[i].max + 1;
                }
        }
        if (IPLIMIT > current) {
                result += IPLIMIT - current;
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
