#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

struct range {
        int start;
        int end;
};

struct pair {
        struct range elf1;
        struct range elf2;
};

static int parse_integer(const char **const input) {
        char c;
        int r = 0;
        ASSERT(isdigit(**input), "parse error");
        while (isdigit(c = **input)) {
                r *= 10;
                r += c - '0';
                *input += 1;
        }
        return r;
}

static void parse_range(const char **const input, struct range *const range) {
        range->start = parse_integer(input);
        ASSERT(**input == '-', "parse error");
        *input += 1;
        range->end = parse_integer(input);
}

static void parse_pair(const char **const input, struct pair *const pair) {
        parse_range(input, &pair->elf1);
        ASSERT(**input == ',', "parse error");
        *input += 1;
        parse_range(input, &pair->elf2);
}

static bool parse_input(const char **const input, struct pair *const pair) {
        if (**input == '\0') {
                return false;
        }
        parse_pair(input, pair);
        if (**input == '\n') {
                *input += 1;
        } else {
                ASSERT(**input == '\0', "parse error");
        }
        return true;
}

// range1 is fully contained within range2
// |---------------| r2
//       |---|       fully contained: r1.s > r2.s && r1.e < r2.e
static bool range_fully_contained(const struct range *const range1, const struct range *const range2) {
        return range1->start >= range2->start && range1->end <= range2->end;
}

static bool fully_contained(const struct pair *const pair) {
        return range_fully_contained(&pair->elf1, &pair->elf2) ||
                range_fully_contained(&pair->elf2, &pair->elf1);
}

//     |-------------|   r1
//   |---|               overlaps because r1 starts inside: r1.s > r2.s && r1.s < r2.e
//                 |---| overlaps because r1 ends inside:   r1.e > r2.s && r1.e < r2.e
//          |---|        overlaps because fully contained:  see range_fully_contained()
static bool line_overlap(const struct range *const range1, const struct range *const range2) {
        return (range1->start >= range2->start && range1->start <= range2->end) ||
               (range1->end >= range2->start && range1->end <= range2->end) ||
               range_fully_contained(range2, range1);
}

static bool overlaps(const struct pair *const pair) {
        return line_overlap(&pair->elf1, &pair->elf2);
}

static void solution(const char *input, char *const output, bool(*predicate)(const struct pair*)) {
        struct pair pair;
        int count = 0;
        while (parse_input(&input, &pair)) {
                if (predicate(&pair)) {
                        count++;
                }
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static void solution1(const char *input, char *const output) {
        solution(input, output, fully_contained);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, overlaps);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
