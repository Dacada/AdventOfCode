#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX(x, y) (((x)>(y))?(x):(y))
#define MIN(x, y) (((x)<(y))?(x):(y))

#define NOPS 14

struct pair {
        int first, second;
};

static void skip_txt(const char **const input, const char *const txt) {
        size_t len = strlen(txt);
        ASSERT(strncmp(*input, txt, len) == 0, "parse error '%s' '%s'", *input, txt);
        *input += len;
}

static void skip_line(const char **const input) {
        while (**input != '\n') {
                *input += 1;
        }
        *input += 1;
}

static int parse_number(const char **const input) {
        bool neg = false;
        if (**input == '-') {
                neg = true;
                *input += 1;
        }
        ASSERT(isdigit(**input), "parse error");

        int n=0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }

        if (neg) {
                return -n;
        }
        return n;
}

static void parse_input(const char *input, struct pair pairs[NOPS]) {
        for (int i=0; i<NOPS; i++) {
                skip_txt(&input, "inp w");
                for (int j=0; j<5; j++) {
                        skip_line(&input);
                }
                skip_txt(&input, "add x ");
                pairs[i].first = parse_number(&input);
                for (int j=0; j<10; j++) {
                        skip_line(&input);
                }
                skip_txt(&input, "add y ");
                pairs[i].second = parse_number(&input);
                for (int j=0; j<3; j++) {
                        skip_line(&input);
                }
        }
}

struct stack {
        struct pair elements[NOPS];
        int top;
};

static void stack_init(struct stack *const stack) {
        stack->top = 0;
}
static void stack_push(struct stack *const stack, struct pair element) {
        ASSERT(stack->top < NOPS, "stack full");
        stack->elements[stack->top++] = element;
}
static struct pair stack_pop(struct stack *const stack) {
        ASSERT(stack->top > 0, "stack empty");
        return stack->elements[--stack->top];
}

struct set {
        struct pair elements[NOPS];
        bool filled[NOPS];
};

static void set_init(struct set *const set) {
        for (int i=0; i<NOPS; i++) {
                set->filled[i] = false;
        }
}

static void set_add(struct set *const set, int idx, struct pair element) {
        set->elements[idx] = element;
        set->filled[idx] = true;
}

static bool set_get(struct set *const set, int idx, struct pair *element) {
        *element = set->elements[idx];
        return set->filled[idx];
}

static void solution(const char *const input, char *const output, bool minimize) {
        struct pair pairs[NOPS];
        parse_input(input, pairs);

        struct stack stack;
        stack_init(&stack);

        struct set links;
        set_init(&links);

        for (int i=0; i<NOPS; i++) {
                int a = pairs[i].first;
                int b = pairs[i].second;

                if (a > 0) {
                        stack_push(&stack, (struct pair){.first=i, .second=b});
                } else {
                        struct pair p = stack_pop(&stack);
                        int j = p.first;
                        int bj = p.second;
                        set_add(&links, i, (struct pair){.first=j, .second=bj+a});
                }
        }

        int assignments[14];

        for (int i=0; i<NOPS; i++) {
                struct pair p;
                if (set_get(&links, i, &p)) {
                        int j = p.first;
                        int delta = p.second;

                        int val1;
                        int val2;
                        if (minimize) {
                                val1 = MAX(1, 1+delta);
                                val2 = MAX(1, 1-delta);
                        } else {
                                val1 = MIN(9, 9+delta);
                                val2 = MIN(9, 9-delta);
                        }

                        assignments[i] = val1;
                        assignments[j] = val2;
                }
        }

        char solution[NOPS+1];
        for (int i=0; i<NOPS; i++) {
                solution[i] = assignments[i]+'0';
        }
        solution[NOPS] = '\0';

        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", solution);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, false);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, true);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
