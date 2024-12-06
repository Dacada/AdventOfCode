#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 100

static unsigned count_safe_tiles(const char *const row) {
  unsigned count = 0;
  for (int i = 0; i < WIDTH; i++) {
    count += row[i] == '.';
  }
  return count;
}

static char is_safe(char left, char right) {
  // more optimal, less comparisons?
  if (((left == '^' && right == '.') || (left == '.' && right == '^'))) {
    return '^';
  } else {
    return '.';
  }
}

static void advance_row(const char *const prev, char *const next) {
  for (int i = 0; i < WIDTH; i++) {
    char left = i == 0 ? '.' : prev[i - 1];
    char right = i == WIDTH - 1 ? '.' : prev[i + 1];

    next[i] = is_safe(left, right);
  }
}

static void solution(const char *const input, char *const output, unsigned rows) {
  char buff1[WIDTH + 1];
  char buff2[WIDTH + 1];

  strncpy(buff1, input, WIDTH);

  char *row = buff1;
  char *other_row = buff2;

  unsigned count = 0;
  for (unsigned i = 0; i < rows; i++) {
    count += count_safe_tiles(row);
    advance_row(row, other_row);

    char *tmp = row;
    row = other_row;
    other_row = tmp;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
}

static void solution1(const char *const input, char *const output) { solution(input, output, 40); }

static void solution2(const char *const input, char *const output) { solution(input, output, 400000); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
