#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define NUM_COUNT 25

static unsigned parse_number(const char *const input, size_t *const idx) {
        unsigned num = 0;
        char c;
        while (isdigit(c = input[*idx])) {
                num = num*10 + c-'0';
                *idx += 1;
        }
        return num;
}

static unsigned *all_numbers = NULL;
static size_t all_numbers_capacity;
static size_t all_numbers_length;
static void parse(const char *const input) {
        for (size_t i=0; input[i]!='\0'; i++) {
                unsigned num = parse_number(input, &i);
                ASSERT(input[i] == '\n', "parse error");

                if (all_numbers_length >= all_numbers_capacity) {
                        if (all_numbers_capacity == 0) {
                                all_numbers_capacity = 256;
                        } else {
                                all_numbers_capacity *= 2;
                        }
                        all_numbers = realloc(all_numbers, sizeof(unsigned)*all_numbers_capacity);
                }
                all_numbers[all_numbers_length] = num;
                all_numbers_length++;
        }
}

static unsigned last_number_idx;
static unsigned sums[NUM_COUNT*NUM_COUNT];
static unsigned numbers[NUM_COUNT];
static void add_number(unsigned n, bool update) {
        for (size_t i=0; i<NUM_COUNT; i++) {
                if (update) {
                        sums[i*NUM_COUNT+last_number_idx] -= numbers[last_number_idx];
                        sums[i*NUM_COUNT+last_number_idx] += n;
                        sums[last_number_idx*NUM_COUNT+i] -= numbers[last_number_idx];
                        sums[last_number_idx*NUM_COUNT+i] += n;
                } else {
                        sums[i*NUM_COUNT+last_number_idx] += n;
                        sums[last_number_idx*NUM_COUNT+i] += n;
                }
        }
        numbers[last_number_idx] = n;
        last_number_idx = (last_number_idx+1) % NUM_COUNT;
}
static bool is_correct(unsigned num) {
        for (size_t i=0; i<NUM_COUNT; i++) {
                for (size_t j=0; j<NUM_COUNT; j++) {
                        if (i != j) {
                                if (num == sums[j*NUM_COUNT+i]) {
                                        return true;
                                }
                        }
                }
        }
        return false;
}

static unsigned solution1_result() {
        unsigned num = 0;
        int j=0;
        for (size_t i=0; i<all_numbers_length; i++) {
                num = all_numbers[i];
                if (j>=25 && !is_correct(num)) {
                        break;
                }
                add_number(num, j>=25);
                j++;
        }
        return num;
}

static unsigned solution2_result(unsigned goal) {
        for (size_t start=0; start<all_numbers_length; start++) {
                unsigned min, max, sum;
                min = max = sum = all_numbers[start];
                for (size_t i=start+1; i<all_numbers_length; i++) {
                        unsigned num = all_numbers[i];
                        min = num < min ? num : min;
                        max = num > max ? num : max;
                        sum += num;
                        if (sum == goal) {
                                return min + max;
                        } else if (sum > goal) {
                                break;
                        }
                }
        }
        return 0;
}

static void solution1(const char *const input, char *const output) {
        parse(input);
        unsigned result = solution1_result();
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
        free(all_numbers);
}

static void solution2(const char *const input, char *const output) {
        parse(input);
        unsigned goal = solution1_result();
        unsigned result = solution2_result(goal);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
        free(all_numbers);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
