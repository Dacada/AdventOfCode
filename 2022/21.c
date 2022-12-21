#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

enum monkey_type {
        MONKEY_NUMBER,
        MONKEY_OPERATION,
};

enum monkey_operation {
        OPERATION_ADDITION,
        OPERATION_SUBTRACTION,
        OPERATION_MULTIPLICATION,
        OPERATION_DIVISION,
};

struct monkey {
        enum monkey_type type;
        union {
                int number;
                struct {
                        int monkey1;
                        int monkey2;
                        bool idx1;
                        bool idx2;
                        enum monkey_operation operation;
                } operation;
        };
};

static void skip_newlines(const char **input) {
        while (**input == '\n') {
                *input += 1;
        }
}

static int parse_int(const char **input) {
        int r = 0;
        while (isdigit(**input)) {
                r *= 10;
                r += **input - '0';
                *input += 1;
        }
        return r;
}

static int parse_monkey_name(const char **input) {
        static char *seen[1<<10] = {"root"};
        static int nseen = 1;

        int i;
        for (i=0; i<nseen; i++) {
                if (strncmp(*input, seen[i], 4) == 0) {
                        goto end;
                }
        }
        seen[i] = strndup(*input, 4);

end:
        *input += 4;
        return i;
}

static void parse_monkey(const char **input, struct monkey *monkey) {
        ASSERT(**input == ':', "parse error");
        *input += 1;
        ASSERT(**input == ' ', "parse error");
        *input += 1;

        if (isdigit(**input)) {
                monkey->type = MONKEY_NUMBER;
                monkey->number = parse_int(input);
                return;
        }

        monkey->type = MONKEY_OPERATION;
        monkey->operation.idx1 = true;
        monkey->operation.idx2 = true;
        monkey->operation.monkey1 = parse_monkey_name(input);
        ASSERT(**input == ' ', "parse error");
        *input += 1;

        switch(**input) {
        case '+':
                monkey->operation.operation = OPERATION_ADDITION;
                break;
        case '-':
                monkey->operation.operation = OPERATION_SUBTRACTION;
                break;
        case '*':
                monkey->operation.operation = OPERATION_MULTIPLICATION;
                break;
        case '/':
                monkey->operation.operation = OPERATION_DIVISION;
                break;
        default:
                FAIL("parse error");
        }
        *input += 1;

        ASSERT(**input == ' ', "parse error");
        *input += 1;
        monkey->operation.monkey2 = parse_monkey_name(input);
}

static int parse_input(const char *input, struct monkey **monkeys) {
        int len = 1;
        int cap = 16;
        *monkeys = malloc(sizeof(**monkeys)*cap);

        while (*input != '\0') {
                if (len >= cap) {
                        cap *= 2;
                        *monkeys = realloc(*monkeys, sizeof(**monkeys)*cap);
                }
                int i = parse_monkey_name(&input);
                parse_monkey(&input, *monkeys + i);
                skip_newlines(&input);
                if (i != 0) {
                        len++;
                }
        }

        return len;
}

static int operate(enum monkey_operation operation, int a, int b) {
        switch(operation) {
        case OPERATION_ADDITION:
                return a + b;
        case OPERATION_SUBTRACTION:
                return a - b;
        case OPERATION_MULTIPLICATION:
                return a * b;
        case OPERATION_DIVISION:
                return a / b;
        default:
                FAIL("invalid operation");
        }
}

static void solution1(const char *const input, char *const output) {
        struct monkey *monkeys;
        int len = parse_input(input, &monkeys);

        int *needed_by = malloc(sizeof(*needed_by)*len*len);
        for (int i=0; i<len; i++) {
                needed_by[i*len + 0] = 1;
        }
        for (int i=0; i<len; i++) {
                struct monkey *m = &monkeys[i];
                if (m->type == MONKEY_OPERATION) {
                        DBG("%d needs %d", m->operation.monkey1, i);
                        int j = m->operation.monkey1;
                        int size = needed_by[j*len + 0];
                        needed_by[j*len + size] = i;
                        needed_by[j*len + 0]++;

                        DBG("%d needs %d", m->operation.monkey2, i);
                        j = m->operation.monkey2;
                        size = needed_by[j*len + 0];
                        needed_by[j*len + size] = i;
                        needed_by[j*len + 0]++;
                }
        }

        bool done = false;
        int cock = 0;
        while (!done) {
                done = true;
                for (int i=0; i<len; i++) {
                        struct monkey *m = &monkeys[i];
                        if (m->type == MONKEY_NUMBER) {
                                int size = needed_by[i*len + 0];
                                for (int j=1; j<size; j++) {
                                        int k = needed_by[i*len + j];
                                        struct monkey *mm = &monkeys[k];
                                        ASSERT(mm->type == MONKEY_OPERATION, "should be oepration");
                                        if (mm->operation.idx1 && mm->operation.monkey1 == i) {
                                                mm->operation.idx1 = false;
                                                mm->operation.monkey1 = m->number;
                                        } else if (mm->operation.idx2 && mm->operation.monkey2 == i) {
                                                mm->operation.idx2 = false;
                                                mm->operation.monkey2 = m->number;
                                        } else {
                                                FAIL("not needed by");
                                        }
                                        if (!mm->operation.idx1 && !mm->operation.idx2) {
                                                DBG("%d",cock++);
                                                int r = operate(mm->operation.operation, mm->operation.monkey1, mm->operation.monkey2);
                                                mm->type = MONKEY_NUMBER;
                                                mm->number = r;
                                        }
                                }
                        } else {
                                done = false;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", monkeys[0].number);
        free(monkeys);
        free(needed_by);
}

static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
