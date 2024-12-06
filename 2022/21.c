#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

enum monkey_type {
  MONKEY_NUMBER,
  MONKEY_FAKE_NUMBER,
  MONKEY_OPERATION,
  MONKEY_DONE,
  MONKEY_FAKE_DONE,
};

enum monkey_operation {
  OPERATION_ADDITION,
  OPERATION_SUBTRACTION,
  OPERATION_MULTIPLICATION,
  OPERATION_DIVISION,
};

enum monkey_operand_type {
  OPERAND_REFERENCE,
  OPERAND_NUMBER,
  OPERAND_FAKE_NUMBER,
};

struct monkey_operand {
  enum monkey_operand_type type;
  long value;
};

struct monkey {
  enum monkey_type type;
  union {
    long number;
    struct {
      struct monkey_operand operand1;
      struct monkey_operand operand2;
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

int human;
static char *seen_names[1 << 11] = {"root"};
static int parse_monkey_name(const char **input) {
  static int nseen = 1;

  int i;
  for (i = 0; i < nseen; i++) {
    if (strncmp(*input, seen_names[i], 4) == 0) {
      goto end;
    }
  }
  seen_names[i] = strndup(*input, 4);
  if (strcmp(seen_names[i], "humn") == 0) {
    human = i;
  }
  nseen++;

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
  monkey->operation.operand1.type = OPERAND_REFERENCE;
  monkey->operation.operand1.value = parse_monkey_name(input);
  ASSERT(**input == ' ', "parse error");
  *input += 1;

  switch (**input) {
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
  monkey->operation.operand2.type = OPERAND_REFERENCE;
  monkey->operation.operand2.value = parse_monkey_name(input);
}

static int parse_input(const char *input, struct monkey **monkeys) {
  int len = 0;
  int cap = 16;
  *monkeys = malloc(sizeof(**monkeys) * cap);

  while (*input != '\0') {
    int i = parse_monkey_name(&input);
    if (i + 1 > len) {
      len = i + 1;
    }
    if (len >= cap) {
      cap *= 2;
      *monkeys = realloc(*monkeys, sizeof(**monkeys) * cap);
    }
    parse_monkey(&input, *monkeys + i);
    skip_newlines(&input);
  }

  return len;
}

static long operate(enum monkey_operation operation, long a, long b) {
  switch (operation) {
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

static void resolve(struct monkey *monkeys, int len, bool part2) {
  int *needed_by = malloc(sizeof(*needed_by) * len * len);
  for (int i = 0; i < len; i++) {
    needed_by[i * len + 0] = 1;
  }
  for (int i = 0; i < len; i++) {
    struct monkey *m = &monkeys[i];
    if (m->type == MONKEY_OPERATION) {
      ASSERT(m->operation.operand1.type == OPERAND_REFERENCE, "expected reference operand");
      int j = m->operation.operand1.value;
      int size = needed_by[j * len + 0];
      needed_by[j * len + size] = i;
      needed_by[j * len + 0]++;

      ASSERT(m->operation.operand2.type == OPERAND_REFERENCE, "expected reference operand");
      j = m->operation.operand2.value;
      size = needed_by[j * len + 0];
      needed_by[j * len + size] = i;
      needed_by[j * len + 0]++;
    }
  }

  if (part2) {
    monkeys[human].type = MONKEY_FAKE_NUMBER;
    monkeys[human].operation.operand1.type = OPERAND_FAKE_NUMBER;
    monkeys[human].operation.operand1.value = -1;
    monkeys[human].operation.operand2.type = OPERAND_FAKE_NUMBER;
    monkeys[human].operation.operand2.value = -1;
  }

  bool done = false;
  while (!done) {
    done = true;
    for (int i = 0; i < len; i++) {
      struct monkey *m = &monkeys[i];
      if (m->type == MONKEY_DONE || m->type == MONKEY_FAKE_DONE) {
        continue;
      }
      if (m->type == MONKEY_NUMBER || m->type == MONKEY_FAKE_NUMBER) {
        int size = needed_by[i * len + 0];
        for (int j = 1; j < size; j++) {
          int k = needed_by[i * len + j];
          struct monkey *mm = &monkeys[k];
          ASSERT(mm->type == MONKEY_OPERATION, "should be oepration not %d", mm->type);
          if (mm->operation.operand1.type == OPERAND_REFERENCE && mm->operation.operand1.value == i) {
            if (m->type == MONKEY_NUMBER) {
              if (k == 0) {
                DBG("operator 1 of root is %ld", m->number);
              }
              mm->operation.operand1.type = OPERAND_NUMBER;
              mm->operation.operand1.value = m->number;
            } else {
              mm->operation.operand1.type = OPERAND_FAKE_NUMBER;
              mm->operation.operand1.value = i;
            }
          } else if (mm->operation.operand2.type == OPERAND_REFERENCE && mm->operation.operand2.value == i) {
            if (m->type == MONKEY_NUMBER) {
              if (k == 0) {
                DBG("operator 2 of root is %ld", m->number);
              }
              mm->operation.operand2.type = OPERAND_NUMBER;
              mm->operation.operand2.value = m->number;
            } else {
              mm->operation.operand2.type = OPERAND_FAKE_NUMBER;
              mm->operation.operand2.value = i;
            }
          } else {
            FAIL("%d should be needed by %d yet %d only needs %ld(type=%d) "
                 "and %ld(type=%d)",
                 i, k, k, mm->operation.operand1.value, mm->operation.operand1.type, mm->operation.operand2.value,
                 mm->operation.operand2.type);
          }
          if ((mm->operation.operand1.type == OPERAND_NUMBER || mm->operation.operand1.type == OPERAND_FAKE_NUMBER) &&
              (mm->operation.operand2.type == OPERAND_NUMBER || mm->operation.operand2.type == OPERAND_FAKE_NUMBER)) {
            if (mm->operation.operand1.type == OPERAND_NUMBER && mm->operation.operand2.type == OPERAND_NUMBER) {
              long r = operate(mm->operation.operation, mm->operation.operand1.value, mm->operation.operand2.value);
              mm->type = MONKEY_NUMBER;
              mm->number = r;
            } else {
              mm->type = MONKEY_FAKE_NUMBER;
            }
          }
        }
        if (m->type == MONKEY_NUMBER) {
          m->type = MONKEY_DONE;
        } else {
          m->type = MONKEY_FAKE_DONE;
        }
      } else {
        done = false;
      }
    }
  }
  free(needed_by);
}

static void solution1(const char *const input, char *const output) {
  struct monkey *monkeys;
  int len = parse_input(input, &monkeys);

#ifdef DEBUG
  for (int i = 0; i < len; i++) {
    fprintf(stderr, "%d: ", i);
    if (monkeys[i].type == MONKEY_OPERATION) {
      fprintf(stderr, "%ld %c %ld\n", monkeys[i].operation.operand1.value, "+-*/"[monkeys[i].operation.operation],
              monkeys[i].operation.operand2.value);
    } else {
      fprintf(stderr, "%ld\n", monkeys[i].number);
    }
  }
#endif

  resolve(monkeys, len, false);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", monkeys[0].number);
  free(monkeys);
}

static long reverse_operate(long operand, long result, bool first_operand, enum monkey_operation operation) {
  switch (operation) {
  case OPERATION_ADDITION:
    DBG("a %ld + %ld = %ld", operand, result - operand, result);
    return result - operand;
  case OPERATION_SUBTRACTION:
    if (first_operand) {
      DBG("b %ld - %ld = %ld", operand, operand - result, result);
      return operand - result;
    } else {
      DBG("c %ld - %ld = %ld", result + operand, operand, result);
      return result + operand;
    }
  case OPERATION_MULTIPLICATION:
    DBG("d %ld * %ld = %ld", operand, result / operand, result);
    ASSERT(result % operand == 0, "bad division!");
    return result / operand;
  case OPERATION_DIVISION:
    if (first_operand) {
      DBG("e %ld / %ld = %ld", operand, operand / result, result);
      ASSERT(operand % result == 0, "bad division!");
      return operand / result;
    } else {
      DBG("f %ld / %ld = %ld", result * operand, operand, result);
      return result * operand;
    }
  default:
    FAIL("invalid operation");
  }
}

static long make_monkey_output(int monkey, long value, const struct monkey *monkeys) {
  DBG("Monkey %s should output %ld", seen_names[monkey], value);
  const struct monkey *m = monkeys + monkey;

  if (m->operation.operand1.type == OPERAND_FAKE_NUMBER && m->operation.operand2.type == OPERAND_FAKE_NUMBER) {
    return value;
  }

  ASSERT(m->type == MONKEY_FAKE_DONE, "bad type");

  long goal;
  int next;
  if (m->operation.operand1.type == OPERAND_NUMBER) {
    long v = m->operation.operand1.value;
    next = m->operation.operand2.value;
    goal = reverse_operate(v, value, true, m->operation.operation);
  } else if (m->operation.operand2.type == OPERAND_NUMBER) {
    next = m->operation.operand1.value;
    long v = m->operation.operand2.value;
    goal = reverse_operate(v, value, false, m->operation.operation);
  } else {
    FAIL("no goal");
  }

  return make_monkey_output(next, goal, monkeys);
}

static void solution2(const char *const input, char *const output) {
  struct monkey *monkeys;
  int len = parse_input(input, &monkeys);

  resolve(monkeys, len, true);

  monkeys[0].operation.operation = OPERATION_SUBTRACTION;
  long res = make_monkey_output(0, 0, monkeys);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
  free(monkeys);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
