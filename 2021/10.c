#include <aoclib.h>
#include <stdio.h>

#define STACKSIZE 256

struct stack {
        char data[STACKSIZE];
        size_t head;
};

static void stack_init(struct stack *const s) {
        s->head = 0;
}

static bool stack_empty(struct stack *const s) {
        return s->head == 0;
}

static void stack_push(struct stack *const s, char c) {
        s->data[s->head] = c;
        s->head++;
        ASSERT(s->head < STACKSIZE, "stack full");
}

static char stack_pop(struct stack *const s) {
        ASSERT(!stack_empty(s), "stack empty");
        s->head--;
        return s->data[s->head];
}

static bool parenmatch(char a, char b) {
        return  (a == '(' && b == ')') ||
                (a == '[' && b == ']') ||
                (a == '{' && b == '}') ||
                (a == '<' && b == '>');
}

static unsigned get_score1(char c) {
        switch (c) {
        case ')':
                return 3;
        case ']':
                return 57;
        case '}':
                return 1197;
        case '>':
                return 25137;
        default:
                ASSERT(false, "invalid character '%c'", c);
                return 0;
        }
}

static unsigned get_score2(char c) {
        switch (c) {
        case '(':
                return 1;
        case '[':
                return 2;
        case '{':
                return 3;
        case '<':
                return 4;
        default:
                ASSERT(false, "invalid character '%c'", c);
                return 0;
        }
}

static int cmp(const void *a, const void *b) {
        const unsigned long *n1 = a;
        const unsigned long *n2 = b;

        if (*n1 > *n2) {
                return 1;
        } else if (*n1 < *n2) {
                return -1;
        } else {
                return 0;
        }
}

static void solution(const char *const input, unsigned long *const scores) {
        struct stack s;
        stack_init(&s);

        unsigned score1 = 0;
        
        unsigned long score2[256];
        unsigned line = 0;
        
        for (size_t i=0; input[i] != '\0'; i++) {
                char c = input[i];
                if (c == '\n') {
                        unsigned long linescore = 0;
                        while (!stack_empty(&s)) {
                                linescore *= 5;
                                linescore += get_score2(stack_pop(&s));
                        }
                        score2[line++] = linescore;
                        stack_init(&s);
                } else if (c == '(' || c == '[' || c == '{' || c == '<') {
                        stack_push(&s, c);
                } else if (c == ')' || c == ']' || c == '}' || c == '>') {
                        if (stack_empty(&s)) {
                                // missmatched
                                ASSERT(false, "this should never happen, %s", input+i);
                        } else {
                                char d = stack_pop(&s);
                                if (parenmatch(d, c)) {
                                        // match
                                } else {
                                        // incorrect match
                                        score1 += get_score1(c);
                                        while (input[++i] != '\n');
                                        stack_init(&s);
                                }
                        }
                } else {
                        ASSERT(false, "invalid character '%c'", c);
                }
        }

        scores[0] = score1;

        qsort(score2, line, sizeof(*score2), cmp);
        scores[1] = score2[line/2];
        for (size_t i=0; i<line; i++) {
                DBG("%lu", score2[i]);
        }
}

static void solution1(const char *const input, char *const output) {
        unsigned long scores[2];
        solution(input, scores);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", scores[0]);
}

static void solution2(const char *const input, char *const output) {
        unsigned long scores[2];
        solution(input, scores);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", scores[1]);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
