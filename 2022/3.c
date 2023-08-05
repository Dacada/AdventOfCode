#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define TOTAL_ITEMS 52  // A-Za-z

// items are an array the size of all possible items and whether we have item i
struct rucksack {
        bool items1[TOTAL_ITEMS];
        bool items2[TOTAL_ITEMS];
        int size;
};

__attribute__((pure))
static int parse_item(const char item) {
        if (item >= 'a' && item <= 'z') {
                return item - 'a';
        } else if (item >= 'A' && item <= 'Z') {
                return item - 'A' + 26;
        } else {
                FAIL("parse error");
        }
}

static bool parse_rucksack(const char **const input, struct rucksack *const rucksack) {
        if (**input == '\0') {
                return false;
        }
        memset(rucksack, 0, sizeof(*rucksack));

        int i;
        for (i=0;; i++) {
                char c = (*input)[i];
                if (c == '\n' || c == '\0') {
                        break;
                }
        }
        rucksack->size = i;

        for (i=0; i<rucksack->size/2; i++) {
                rucksack->items1[parse_item((*input)[i])] = true;
        }
        for (i=rucksack->size/2; i<rucksack->size; i++) {
                rucksack->items2[parse_item((*input)[i])] = true;
        }
        
        *input += rucksack->size;
        if (**input == '\n') {
                *input += 1;
        }
        return true;
}

static void update_common(const struct rucksack *const rucksack, bool common[TOTAL_ITEMS]) {
        for (int i=0; i<TOTAL_ITEMS; i++) {
                common[i] = common[i] && (rucksack->items1[i] || rucksack->items2[i]);
        }
}

static int find_common(const struct rucksack *const rucksack) {
        int total = 0;
        for (int i=0; i<TOTAL_ITEMS; i++) {
                if (rucksack->items1[i] && rucksack->items2[i]) {
                        total += i + 1;  
                }
        }
        return total;
}

static void solution1(const char *input, char *const output) {
        static struct rucksack rucksack;
        int total = 0;
        while (parse_rucksack(&input, &rucksack)) {
                int common = find_common(&rucksack);
                total += common;
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

static void solution2(const char *input, char *const output) {
        static struct rucksack rucksack;
        static bool common[TOTAL_ITEMS];
        int total = 0;
        for(;;) {
                memset(common, 1, sizeof(common));

                bool done = false;
                for (int i=0; i<3; i++) {
                        done = !parse_rucksack(&input, &rucksack);
                        if (done) {
                                break;
                        }
                        update_common(&rucksack, common);
                }
                if (done) {
                        break;
                }

                for (int i=0; i<TOTAL_ITEMS; i++) {
                        if (common[i]) {
                                total += i + 1;
                                break;
                        }
                }
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
