#include <aoclib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

static const char *parse_number(const char *input, int *const num) {
        int n = 0;
        while (*input >= '0' && *input <= '9') {
                n = (n*10) + *input - '0';
                input++;
        }
        *num = n;
        return input+1;
}

static int *parse_input(const char *input, size_t *const len) {
        size_t cap=32;
        *len = 0;
        
        int *nums = malloc(sizeof(int) * cap);
        
        while (*input != '\0') {
                input = parse_number(input, &nums[*len]);

                (*len)++;

                while (*len >= cap) {
                        cap *= 2;
                        nums = realloc(nums, sizeof(int) * cap);
                }
        }

        nums = realloc(nums, sizeof(int) * *len);
        return nums;
}

static long mult_array(const int *const group, size_t len) {
        long result = 1;
        for (size_t i=0; i<len; i++) {
                result *= group[i];
        }
        return result;
}

static int sum_array(const int *const group, size_t len) {
        int result = 0;
        for (size_t i=0; i<len; i++) {
                result += group[i];
        }
        return result;
}

// Asumes array and subarray are sorted
static int *get_remainder(const int *const array, size_t len,
                          const int *const subarray, size_t sublen,
                          size_t *const rlen) {
        *rlen = len - sublen;
        int *result = malloc(sizeof(int) * (*rlen));
        
        size_t i=0,j=0;
        for (size_t k=0; k<*rlen; k++) {
                while (array[i] == subarray[j]) {
                        i++;
                        j++;
                }
                result[k] = array[i++];
        }

        return result;
}

struct callback_arguments {
        bool found;
        int goal;
        
        const int *numbers;
        size_t len, group_len;

        int **groups;
        size_t *lens;
        size_t ngroups;
};

static void find_arrangement(int *group, void *vargs);

static bool optimal_arrangement(const int *const numbers, size_t len,
                                int **const groups, size_t *const lens,
                                size_t ngroups, int goal) {
        if (ngroups == 1) {
                *groups = malloc(sizeof(int)*len);
                memcpy(groups[0], numbers, sizeof(int)*len);
                *lens = len;
                return true;
        }

        struct callback_arguments args;
        args.goal = goal;
        
        args.numbers = numbers;
        args.len = len;

        args.groups = groups;
        args.lens = lens;
        args.ngroups = ngroups;

        for (size_t thislen=1; thislen+ngroups-1<len; thislen++) {
                args.group_len = thislen;
                args.found = false;
                aoc_combinations(numbers, len, thislen, find_arrangement, &args);
                if (args.found) {
                        return true;
                }
        }

        return false;
}

static void find_arrangement(int *group, void *vargs) {
        struct callback_arguments *args = vargs;

        if (args->found) {
                return;
        }
        
        int sum = sum_array(group, args->group_len);
        if (sum != args->goal) {
                return;
        }

        *(args->groups) = malloc(sizeof(int)*args->group_len);
        memcpy(*(args->groups), group, sizeof(int)*args->group_len);
        *(args->lens) = args->group_len;

        size_t remainder_len;
        int *remainder = get_remainder(args->numbers, args->len, group, args->group_len, &remainder_len);
        
        if (optimal_arrangement(remainder, remainder_len,
                                args->groups+1, args->lens+1,
                                args->ngroups-1, args->goal)) {
                args->found = true;
        }
        free(remainder);
}

static void solution(const char *const input, char *const output, size_t ngroups) {
        size_t lens[ngroups], len;
        int *groups[ngroups], *numbers, goal;
        
        numbers = parse_input(input, &len);
        goal = sum_array(numbers, len) / ngroups;
        optimal_arrangement(numbers, len, groups, lens, ngroups, goal);

        // Searches in lexicographical order, so the first group found
        // will have the lowest "quantum entanglement"
        
        long result = mult_array(groups[0], lens[0]);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
        
        free(numbers);
        for (size_t i=0; i<ngroups; i++) {
                free(groups[i]);
        }
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 3);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 4);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
