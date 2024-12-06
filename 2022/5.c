#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define STACK_CAPACITY 256

struct stack {
  char elements[STACK_CAPACITY];
  int head;
};

static void stack_init(struct stack *const stack) { stack->head = -1; }

static char stack_top(const struct stack *const stack) {
  ASSERT(stack->head >= 0, "empty stack");
  return stack->elements[stack->head];
}

static char stack_pop(struct stack *const stack) {
  ASSERT(stack->head >= 0, "empty stack");
  return stack->elements[stack->head--];
}

static void stack_push(struct stack *const stack, char element) {
  ASSERT(stack->head < STACK_CAPACITY, "full stack");
  stack->elements[++stack->head] = element;
}

#ifdef DEBUG

static void stack_print(const struct stack *const stack) {
  for (int i = 0; i <= stack->head; i++) {
    fprintf(stderr, "%c ", stack->elements[i]);
  }
}

#endif

static void reverse_array(char *array, int len) {
  for (int i = 0; i < len / 2; i++) {
    int j = len - i - 1;
    char tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
  }
}

static void stack_reverse(struct stack *const stack) { reverse_array(stack->elements, stack->head + 1); }

struct instruction {
  int source;
  int destination;
  int amount;
};

static void apply_instruction_1(const struct instruction *const instr, struct stack *const stacks) {
  struct stack *const source = &stacks[instr->source];
  struct stack *const dest = &stacks[instr->destination];
  for (int i = 0; i < instr->amount; i++) {
    char e = stack_pop(source);
    stack_push(dest, e);
  }
}

static void apply_instruction_2(const struct instruction *const instr, struct stack *const stacks) {
  struct stack *const src = &stacks[instr->source];
  struct stack *const dst = &stacks[instr->destination];
  int amnt = instr->amount;

  ASSERT(src->head - amnt >= 0, "empty stack");
  ASSERT(dst->head + amnt < STACK_CAPACITY, "full stack");

  src->head -= amnt;
  memcpy(dst->elements + dst->head + 1, src->elements + src->head + 1, amnt * sizeof(char));
  dst->head += amnt;
}

static int parse_stack_cell(const char **const input) {
  const char *str = *input;

  int ret;
  if (str[0] == ' ') {
    if (str[1] == ' ') {
      // empty cell
      ASSERT(str[2] == ' ', "parse error");
      ret = 0;
    } else if (str[1] == '1') {
      // last line
      ret = -1;
    } else {
      FAIL("parse error");
    }
  } else if (str[0] == '[') {
    ASSERT(str[2] == ']', "parse error");
    ret = str[1];
  } else {
    FAIL("parse error");
  }

  *input += 3;
  return ret;
}

static bool parse_stack_line(const char **const input, struct stack *const stacks, int len) {
  for (int i = 0; i < len; i++) {
    int c = parse_stack_cell(input);
    if (c == -1) { // last line of stacks with the 1 2 3 numbers
      while (**input != '\n') {
        *input += 1;
      }
      *input += 1;
      return false;
    }
    ASSERT(**input == ' ' || **input == '\n', "parse error");
    *input += 1;

    if (c > 0) { // cell was not empty
      stack_push(&stacks[i], c);
    }
  }
  return true;
}

static int parse_stacks(const char **const input, struct stack **const stacks) {
  int len;
  for (len = 0;; len++) {
    if ((*input)[len] == '\n') {
      break;
    }
  }
  len += 1;
  len /= 4;

  free(*stacks);
  *stacks = malloc(len * sizeof(**stacks));
  for (int i = 0; i < len; i++) {
    stack_init(&(*stacks)[i]);
  }

  while (parse_stack_line(input, *stacks, len))
    ;
  ASSERT(**input == '\n', "parse error");
  *input += 1;

  for (int i = 0; i < len; i++) {
    stack_reverse(&(*stacks)[i]);
  }

  return len;
}

static int parse_int(const char **const input) {
  int n = 0;
  ASSERT(isdigit(**input), "parse error");
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

#define MATCHSTR(input, str)                                                                                           \
  ASSERT(strncmp(*input, str, sizeof(str) - 1) == 0, "parse error");                                                   \
  *input += sizeof(str) - 1;

static bool parse_instructions_line(const char **const input, struct instruction *const instr) {
  MATCHSTR(input, "move ");
  instr->amount = parse_int(input);
  MATCHSTR(input, " from ");
  instr->source = parse_int(input) - 1;
  MATCHSTR(input, " to ");
  instr->destination = parse_int(input) - 1;
  while (**input == '\n') {
    *input += 1;
    if (**input == '\0') {
      return false;
    }
  }
  return true;
}

#undef MATCHSTR

static int parse_instructions(const char **const input, struct instruction **const instructions) {
  int capacity = 16;
  int len = 0;
  free(*instructions);

  *instructions = malloc(capacity * sizeof(**instructions));
  while (parse_instructions_line(input, &(*instructions)[len++])) {
    if (len >= capacity) {
      capacity *= 2;
      *instructions = realloc(*instructions, capacity * sizeof(**instructions));
    }
  }

  return len;
}

static void solution(const char *input, char *const output,
                     void (*apply_instruction)(const struct instruction *, struct stack *)) {
  struct stack *stacks = NULL;
  struct instruction *instructions = NULL;
  int stacks_len = parse_stacks(&input, &stacks);
  int instructions_len = parse_instructions(&input, &instructions);

  for (int i = 0; i < instructions_len; i++) {
#ifdef DEBUG
    for (int j = 0; j < stacks_len; j++) {
      fprintf(stderr, "%d: ", j + 1);
      stack_print(&stacks[j]);
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif
    apply_instruction(&instructions[i], stacks);
  }

  char *result = malloc((stacks_len + 1) * sizeof(char));
  for (int i = 0; i < stacks_len; i++) {
    result[i] = stack_top(&stacks[i]);
  }
  result[stacks_len] = '\0';
  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", result);

  free(result);
  free(stacks);
  free(instructions);
}

static void solution1(const char *const input, char *const output) { solution(input, output, apply_instruction_1); }

static void solution2(const char *const input, char *const output) { solution(input, output, apply_instruction_2); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
