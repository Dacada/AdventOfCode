#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>

enum operation {
  OPERATION_SUM,
  OPERATION_MUL,
};

static void parse_sheet_cell(const char **input, void *parsed, int x, int y, void *args) {
  (void)args;
  (void)x;
  (void)y;

  long *parsed_long = parsed;

  while (**input == ' ') {
    *input += 1;
  }

  if (**input == '+') {
    *parsed_long = OPERATION_SUM;
    *input += 1;
  } else if (**input == '*') {
    *parsed_long = OPERATION_MUL;
    *input += 1;
  } else {
    *parsed_long = aoc_parse_long(input);
  }

  while (**input == ' ') {
    *input += 1;
  }
}

static void parse_input_1(const char **input, long **sheet, long **operations, int *height, int *width) {
  long *grid = aoc_parse_grid(input, parse_sheet_cell, sizeof(long), height, width, NULL);
  *height -= 1;
  *sheet = grid;
  *operations = grid + (*height * *width);
}

static long sum_up(const long *sheet, int column, int height, int width) {
  long total = 0;
  for (int j = 0; j < height; j++) {
    total += sheet[AOC_2D_IDX(column, j, width)];
  }
  return total;
}

static long mul_up(const long *sheet, int column, int height, int width) {
  long total = 1;
  for (int j = 0; j < height; j++) {
    total *= sheet[AOC_2D_IDX(column, j, width)];
  }
  return total;
}

static void solution1(const char *input, char *const output) {
  int height, width;
  long *sheet;
  long *operations;
  parse_input_1(&input, &sheet, &operations, &height, &width);

  long grand_total = 0;
  for (int i = 0; i < width; i++) {
    long result;
    if (operations[i] == OPERATION_SUM) {
      result = sum_up(sheet, i, height, width);
    } else if (operations[i] == OPERATION_MUL) {
      result = mul_up(sheet, i, height, width);
    } else {
      FAIL("invalid operation %ld", operations[i]);
    }
    grand_total += result;
  }

  free(sheet);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", grand_total);
}

static bool consume_column(const char *sheet, int *i, int *j, int width, int height, enum operation *op, long *num) {
  if (*i < 0) {
    return false;
  }
  if (*j != 0) {
    FAIL("parse error");
  }

  bool any = false;
  *num = 0;
  for (; *j < height; *j += 1) {
    char c = sheet[AOC_2D_IDX(*i, *j, width)];
    if (isdigit(c)) {
      *num *= 10;
      *num += c - '0';
      any = true;
    } else if (c == ' ') {
      continue;
    } else if (c == '+') {
      *op = OPERATION_SUM;
    } else if (c == '*') {
      *op = OPERATION_MUL;
    } else {
      FAIL("parse error");
    }
  }

  *j = 0;
  *i -= 1;

  return any;
}

static bool consume_operation(const char *sheet, int *i, int *j, int width, int height, long *result) {
  if (*i < 0) {
    return false;
  }

  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(long), 8);

  enum operation op = -1;
  long num;
  while (consume_column(sheet, i, j, width, height, &op, &num)) {
    long *numptr = aoc_dynarr_grow(&arr, 1);
    *numptr = num;
  }

  // reusing these even though it doesn't exactly fit wiht part 1
  if (op == OPERATION_SUM) {
    *result = sum_up(arr.data, 0, arr.len, 1);
  } else if (op == OPERATION_MUL) {
    *result = mul_up(arr.data, 0, arr.len, 1);
  } else {
    FAIL("invalid operation %d", op);
  }

  aoc_dynarr_free(&arr);

  return true;
}

static void solution2(const char *input, char *const output) {
  int height, width;
  char *sheet = aoc_parse_grid_chars(&input, &height, &width);

  int i = width - 1;
  int j = 0;
  long res;
  long total = 0;
  while (consume_operation(sheet, &i, &j, width, height, &res)) {
    total += res;
  }

  free(sheet);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
