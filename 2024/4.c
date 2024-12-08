#include <aoclib.h>
#include <stdbool.h>
#include <stdio.h>

#define IDX(buff, i, j, cols) ((buff)[(j) * (cols) + (i)])

static inline bool is_xmas(char *buff, int i, int j, int cols, int rows, int di, int dj) {
  if (i + di * 3 < 0 || i + di * 3 >= cols || j + dj * 3 < 0 || j + dj * 3 >= rows) {
    return false;
  }

  if (IDX(buff, i, j, cols) != 'X') {
    return false;
  }

  i += di;
  j += dj;

  if (IDX(buff, i, j, cols) != 'M') {
    return false;
  }

  i += di;
  j += dj;

  if (IDX(buff, i, j, cols) != 'A') {
    return false;
  }

  i += di;
  j += dj;

  if (IDX(buff, i, j, cols) != 'S') {
    return false;
  }

  return true;
}

static inline bool is_x_mas(char *buff, int i, int j, int cols, int rows) {
  if (i - 1 < 0 || i + 1 >= cols || j - 1 < 0 || j + 1 >= rows) {
    return false;
  }

  if (IDX(buff, i, j, cols) != 'A') {
    return false;
  }

  bool first_ok = false;
  if (IDX(buff, i - 1, j - 1, cols) == 'M' && IDX(buff, i + 1, j + 1, cols) == 'S') {
    first_ok = true;
  }

  if (IDX(buff, i + 1, j + 1, cols) == 'M' && IDX(buff, i - 1, j - 1, cols) == 'S') {
    first_ok = true;
  }

  if (!first_ok) {
    return false;
  }

  bool second_ok = false;
  if (IDX(buff, i + 1, j - 1, cols) == 'M' && IDX(buff, i - 1, j + 1, cols) == 'S') {
    second_ok = true;
  }

  if (IDX(buff, i - 1, j + 1, cols) == 'M' && IDX(buff, i + 1, j - 1, cols) == 'S') {
    second_ok = true;
  }

  return second_ok;
}

static char *parse_input(const char *input, int *rows, int *cols) { return aoc_parse_grid_chars(&input, rows, cols); }

static void solution1(const char *const input, char *const output) {
  int rows, cols;
  char *buff = parse_input(input, &rows, &cols);

#ifdef DEBUG
  for (int j = 0; j < rows; j++) {
    for (int i = 0; i < cols; i++) {
      char c = IDX(buff, i, j, cols);
      fprintf(stderr, "%c", c);
    }
    fprintf(stderr, "\n");
  }
#endif

  int total = 0;
  for (int j = 0; j < rows; j++) {
    for (int i = 0; i < cols; i++) {
      total += is_xmas(buff, i, j, cols, rows, 1, 0);
      total += is_xmas(buff, i, j, cols, rows, -1, 0);
      total += is_xmas(buff, i, j, cols, rows, 0, 1);
      total += is_xmas(buff, i, j, cols, rows, 0, -1);

      total += is_xmas(buff, i, j, cols, rows, 1, 1);
      total += is_xmas(buff, i, j, cols, rows, 1, -1);
      total += is_xmas(buff, i, j, cols, rows, -1, 1);
      total += is_xmas(buff, i, j, cols, rows, -1, -1);
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(buff);
}

static void solution2(const char *const input, char *const output) {
  int rows, cols;
  char *buff = parse_input(input, &rows, &cols);

  int total = 0;
  for (int j = 0; j < rows; j++) {
    for (int i = 0; i < cols; i++) {
      total += is_x_mas(buff, i, j, cols, rows);
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(buff);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
