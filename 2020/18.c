#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define STACKSIZE 64

enum token_type {
        INTEGER,
        OPERATOR,
        END,
};

struct token {
        enum token_type type;
        long value;
};

struct stack {
        size_t next;
        struct token elements[STACKSIZE];
};

static void stack_init(struct stack *stack) {
        stack->next = 0;
}

static void stack_push(struct stack *const stack, struct token value) {
        ASSERT(stack->next < STACKSIZE, "push full stack");
        stack->elements[stack->next++] = value;
}

static struct token stack_pop(struct stack *const stack) {
        ASSERT(stack->next > 0, "pop empty stack");
        return stack->elements[--stack->next];
}

static struct token stack_peek(const struct stack *const stack) {
        ASSERT(stack->next > 0, "peek empty stack");
        return stack->elements[stack->next-1];
}

static bool stack_empty(const struct stack *const stack) {
        return stack->next == 0;
}

static long parse_int(const char **const input) {
        long n = 0;
        char c;
        while (isdigit(c=**input)) {
                n=n*10+c-'0';
                *input+=1;
        }
        return n;
}

static struct token *parse_line(const char **const input) {
        size_t capacity = 8;
        struct token *list = malloc(capacity * sizeof *list);
        size_t length = 0;

        for (; **input!='\n'&&**input!='\0'; *input+=1) {
                if (length >= capacity) {
                        capacity *= 2;
                        list = realloc(list, capacity * sizeof *list);
                }
                struct token *t = list+length;

                char c = **input;
                if (isdigit(c)) {
                        t->type = INTEGER;
                        t->value = parse_int(input);
                        *input -= 1;
                        length++;
                } else if (isspace(c)) {
                } else {
                        switch (c) {
                        case '(':
                        case ')':
                        case '+':
                        case '*':
                                t->type = OPERATOR;
                                t->value = c;
                                break;
                        default:
                                FAIL("parse error");
                        }
                        length++;
                }
        }
        if (length >= capacity) {
                capacity += 1;
                list = realloc(list, capacity * sizeof *list);
        }
        list[length].type = END;

        return list;
}

static struct token **parse(const char *input, size_t *length) {
        size_t capacity = 32;
        struct token **list = malloc(capacity * sizeof *list);
        *length = 0;

        while (*input != '\0') {
                if (*length >= capacity) {
                        capacity *= 2;
                        list = realloc(list, capacity * sizeof *list);
                }
                list[*length] = parse_line(&input);
                while (isspace(*input)) {
                        input++;
                }
                *length += 1;
        }

        return list;
}

static void special_push(struct stack *const operators, struct stack *const operands, struct token x) {
        ASSERT(x.type == INTEGER, "special push of non integer");

        if (stack_empty(operands)) {
                stack_push(operands, x);
                return;
        }
        struct token y = stack_peek(operands);
        if (y.type == OPERATOR) {
                ASSERT(y.value == '(', "invalid operator in operand stack");
                stack_push(operands, x);
                return;
        }
        
        ASSERT(!stack_empty(operators), "unexpected empty operators stack");
        
        stack_pop(operands); // already have it as y
        struct token op = stack_pop(operators);

        struct token r;
        r.type = INTEGER;
        switch(op.value) {
        case '+':
                r.value = x.value + y.value;
                break;
        case '*':
                r.value = x.value * y.value;
                break;
        default:
                FAIL("unexpected operand");
        }

        special_push(operators, operands, r);
}

__attribute__((pure))
static long evaluate1(const struct token *token) {
        struct stack operators;
        struct stack operands;
        
        stack_init(&operators);
        stack_init(&operands);

        for (; token->type!=END; token++) {
                switch (token->type) {
                case OPERATOR:
                        if (token->value == '(') {
                                stack_push(&operands, *token);
                        } else if (token->value == ')') {
                                struct token t = stack_pop(&operands);
                                struct token u = stack_pop(&operands);
                                ASSERT(u.type == OPERATOR && u.value == '(', "unbalanced parenthesis");
                                special_push(&operators, &operands, t);
                        } else {
                                stack_push(&operators, *token);
                        }
                        break;
                case INTEGER:
                        special_push(&operators, &operands, *token);
                        break;
                case END:
                        FAIL("end token");
                default:
                        FAIL("invalid token");
                }
        }
        
        struct token result = stack_pop(&operands);
        ASSERT(result.type == INTEGER, "invalid result token");
        return result.value;
}

static void process(struct stack *const operators, struct stack *const operands) {
        struct token x = stack_pop(operands);
        struct token y = stack_pop(operands);
        struct token op = stack_pop(operators);

        struct token r;
        r.type = INTEGER;
        switch(op.value) {
        case '+':
                r.value = x.value + y.value;
                break;
        case '*':
                r.value = x.value * y.value;
                break;
        default:
                FAIL("unexpected operand");
        }

        stack_push(operands, r);
}

__attribute__((pure))
static long evaluate2(const struct token *token) {
        struct stack operators;
        struct stack operands;
        
        stack_init(&operators);
        stack_init(&operands);

        for (; token->type!=END; token++) {
                switch (token->type) {
                case OPERATOR:
                        if (token->value == '(') {
                                stack_push(&operators, *token);
                        } else if (token->value == ')') {
                                struct token t = stack_peek(&operators);
                                while (t.value != '(') {
                                        process(&operators, &operands);
                                        t = stack_peek(&operators);
                                }
                                stack_pop(&operators);
                        } else {
                                if (stack_empty(&operators)) {
                                        stack_push(&operators, *token);
                                } else {
                                        struct token tos = stack_peek(&operators);
                                        if (tos.value == '(' ||
                                            (token->value == '*' && tos.value == '*') ||
                                            (token->value == '+' && tos.value == '+') ||
                                            (token->value == '+' && tos.value == '*')) {
                                                stack_push(&operators, *token);
                                        } else if (token->value == '*' && tos.value == '+') {
                                                while (token->value == '*' && tos.value == '+') {
                                                        process(&operators, &operands);
                                                        if (stack_empty(&operators)) {
                                                                break;
                                                        } else {
                                                                tos = stack_peek(&operators);
                                                        }
                                                }
                                                stack_push(&operators, *token);
                                        } else {
                                                FAIL("invalid operator");
                                        }
                                }
                        }
                        break;
                case INTEGER:
                        stack_push(&operands, *token);
                        break;
                case END:
                        FAIL("end token");
                default:
                        FAIL("invalid token");
                }
        }

        while (!stack_empty(&operators)) {
                process(&operators, &operands);
        }
        
        struct token result = stack_pop(&operands);
        ASSERT(result.type == INTEGER, "invalid result token");
        return result.value;
}

static void solution(const char *const input, char *const output, long(*evaluate)(const struct token*)) {
        size_t len;
        struct token **tokens_lists = parse(input, &len);

        long total = 0;
        for (size_t i=0; i<len; i++) {
                total += evaluate(tokens_lists[i]);
                free(tokens_lists[i]);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
        free(tokens_lists);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, evaluate1);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, evaluate2);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
