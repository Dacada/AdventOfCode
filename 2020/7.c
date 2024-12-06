#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define COLOR_LEN 32

struct bag {
  char color[COLOR_LEN];

  size_t total_parents;
  size_t parents_capacity;
  struct bag **parents;

  size_t total_children;
  size_t children_capacity;
  struct bag **children;
  unsigned *children_amounts;

  int subbags_count;
};

__attribute__((nonnull(1, 2))) static int strcmp_wrapper(const void *a, const void *b) {
  const struct bag *bag_a = a;
  const struct bag *bag_b = b;
  return strcmp(bag_a->color, bag_b->color);
}

__attribute__((nonnull(1))) static void init_bag(struct bag *const bag, const char *color) {
  strcpy(bag->color, color);
  bag->total_parents = 0;
  bag->parents_capacity = 0;
  bag->parents = NULL;
  bag->total_children = 0;
  bag->children = NULL;
  bag->children_capacity = 0;
  bag->children_amounts = NULL;
  bag->subbags_count = -1;
}

__attribute__((nonnull(1))) static void free_bag(struct bag *const bag) {
  free(bag->parents);
  free(bag->children);
  free(bag->children_amounts);
}

__attribute__((nonnull(1, 2))) static void add_child_to_bag(struct bag *const parent, struct bag *const child,
                                                            const unsigned amount) {
  if (parent->total_children >= parent->children_capacity) {
    if (parent->children_capacity == 0) {
      parent->children_capacity = 4;
    } else {
      parent->children_capacity *= 2;
    }
    parent->children = realloc(parent->children, parent->children_capacity * sizeof(child));
    parent->children_amounts = realloc(parent->children_amounts, parent->children_capacity * sizeof(unsigned));
  }
  parent->children[parent->total_children] = child;
  parent->children_amounts[parent->total_children] = amount;
  parent->total_children += 1;

  if (child->total_parents >= child->parents_capacity) {
    if (child->parents_capacity == 0) {
      child->parents_capacity = 2;
    } else {
      child->parents_capacity *= 2;
    }
    child->parents = realloc(child->parents, child->parents_capacity * sizeof(parent));
  }
  child->parents[child->total_parents] = parent;
  child->total_parents += 1;
}

struct stack {
  const struct bag **elements;
  size_t top;
  size_t max_capacity;
};

__attribute__((nonnull(1))) static void stack_init(struct stack *const stack, size_t capacity) {
  stack->elements = malloc(capacity * sizeof(struct bag *));
  stack->max_capacity = capacity;
  stack->top = 0;
}

__attribute__((nonnull(1))) static void stack_push(struct stack *const stack, const struct bag *element) {
  stack->top++;
  ASSERT(stack->top < stack->max_capacity, "push to full stack");
  stack->elements[stack->top] = element;
}

__attribute__((nonnull(1))) static const struct bag *stack_pop(struct stack *const stack) {
  ASSERT(stack->top > 0, "pop from empty stack");
  stack->top--;
  return stack->elements[stack->top + 1];
}

__attribute__((nonnull(1))) static bool stack_empty(const struct stack *const stack) { return stack->top == 0; }

__attribute__((nonnull(1))) static void stack_free(struct stack *const stack) {
  free(stack->elements);
  stack->elements = NULL;
  stack->max_capacity = 0;
}

struct bag *bags = NULL;
size_t bags_capacity = 0;
size_t bags_length = 0;

__attribute__((nonnull(1))) static void add_bag_color(const char *const color) {
  if (bags_length >= bags_capacity) {
    if (bags_capacity == 0) {
      bags_capacity = 16;
    } else {
      bags_capacity *= 2;
    }
    bags = realloc(bags, sizeof(struct bag) * bags_capacity);
  }

  init_bag(bags + bags_length, color);
  bags_length++;
}

__attribute__((nonnull(1))) static struct bag *color_to_bag(const char *const color) {
  if (bags == NULL) {
    return NULL;
  }

  struct bag dummy;
  strcpy(dummy.color, color);
  struct bag *found = bsearch(&dummy, bags, bags_length, sizeof(*bags), strcmp_wrapper);
  return found;
}

__attribute__((nonnull(1, 2))) static void parse_skip(const char *const expected, const char **input) {
  size_t len = strlen(expected);
  ASSERT(strncmp(*input, expected, len) == 0, "parse error");
  *input += len;
}
__attribute__((nonnull(1))) static unsigned parse_number(const char **input) {
  unsigned n = 0;
  while (isdigit(**input)) {
    n = n * 10 + **input - '0';
    (*input)++;
  }
  return n;
}
__attribute__((nonnull(1, 2))) static void parse_color(char *buff, const char **input) {
  memset(buff, 0, sizeof(*buff) * COLOR_LEN);
  bool found_space = false;

  size_t i = 0;
  for (;;) {
    char c = **input;
    if (c == ' ') {
      if (found_space) {
        break;
      } else {
        found_space = true;
      }
    }
    buff[i++] = c;
    (*input)++;
    ASSERT(i < COLOR_LEN, "color len too short");
  }
}
__attribute__((nonnull(1))) static void parse_line_pass(const char **input, const int pass) {
  static char colorbuff[COLOR_LEN];

  parse_color(colorbuff, input);
  struct bag *bag;
  if (pass == 1) {
    bag = NULL;
    add_bag_color(colorbuff);
  } else {
    bag = color_to_bag(colorbuff);
    ASSERT(bag != NULL, "parse_error");
  }

  parse_skip(" bags contain ", input);

  if (**input == 'n') {
    parse_skip("no other bags.", input);
    return;
  }

  for (;;) {
    unsigned amount = parse_number(input);
    ASSERT(**input == ' ', "parse error");
    (*input)++;

    parse_color(colorbuff, input);
    struct bag *child;
    if (pass == 1) {
      child = NULL;
    } else {
      child = color_to_bag(colorbuff);
      ASSERT(child != NULL, "parse_error");
      add_child_to_bag(bag, child, amount);
    }

    parse_skip(" bag", input);
    if (**input == 's') {
      (*input)++;
    }
    if (**input == ',') {
      parse_skip(", ", input);
    } else if (**input == '.') {
      (*input)++;
      break;
    } else {
      FAIL("parse error");
    }
  }
}
__attribute__((nonnull(1))) static void parse_pass(const char *input, const int pass) {
  for (; *input != '\0'; input++) {
    parse_line_pass(&input, pass);
    ASSERT(*input == '\n', "parse error");
  }
}
__attribute__((nonnull(1))) static void parse(const char *const input) {
  parse_pass(input, 1);
  qsort(bags, bags_length, sizeof(*bags), strcmp_wrapper);
  parse_pass(input, 2);
}

__attribute__((nonnull(1))) static unsigned count_total_bags(struct bag *const bag) {
  if (bag->subbags_count > 0) {
    return bag->subbags_count;
  }

  unsigned count = 0;
  for (size_t i = 0; i < bag->total_children; i++) {
    unsigned child_amount = bag->children_amounts[i];
    count += child_amount + child_amount * count_total_bags(bag->children[i]);
  }
  bag->subbags_count = (int)count;
  return count;
}

static void solution1(const char *const input, char *const output) {
  parse(input);

  unsigned count = 0;
  struct bag *shiny_gold = color_to_bag("shiny gold");
  ASSERT(shiny_gold != NULL, "shiny gold bag not found");

  bool bag_seen[bags_length];
  memset(bag_seen, 0, sizeof(bag_seen));

  struct stack stack;
  stack_init(&stack, bags_length);
  stack_push(&stack, shiny_gold);

  while (!stack_empty(&stack)) {
    const struct bag *bag = stack_pop(&stack);
    for (size_t i = 0; i < bag->total_parents; i++) {
      const struct bag *parent = bag->parents[i];
      size_t parent_idx = parent - bags;
      if (!bag_seen[parent_idx]) {
        bag_seen[parent_idx] = true;
        count++;
        stack_push(&stack, parent);
      }
    }
  }

  stack_free(&stack);

  for (size_t i = 0; i < bags_length; i++) {
    free_bag(bags + i);
  }
  free(bags);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
}

static void solution2(const char *const input, char *const output) {
  parse(input);
  unsigned count = count_total_bags(color_to_bag("shiny gold"));
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
