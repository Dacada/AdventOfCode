#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>

static unsigned parse_number(const char **const input) {
  unsigned n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static unsigned *parse_input(const char *input, size_t *const len) {
  size_t size = 16;
  *len = 0;
  unsigned *list = malloc(size * sizeof(*list));

  while (*input != '\0') {
    if (*len >= size) {
      size *= 2;
      list = realloc(list, size * sizeof(*list));
    }
    list[*len] = parse_number(&input);
    *len += 1;

    while (*input == '\n') {
      input += 1;
    }
    ASSERT(*input == ',' || *input == '\0', "parse error");
    if (*input == ',') {
      input += 1;
    }
  }

  return list;
}

static void get_ranges(const unsigned *const nums, const size_t len, int *const start, int *const end) {
  *start = INT_MAX;
  *end = INT_MIN;

  for (size_t i = 0; i < len; i++) {
    if ((int)nums[i] < *start) {
      *start = nums[i];
    }
    if ((int)nums[i] > *end) {
      *end = nums[i];
    }
  }
}

static unsigned get_move_cost_1(const unsigned from, const unsigned to) {
  if (from > to) {
    return from - to;
  } else {
    return to - from;
  }
}

static unsigned get_move_cost_2(const unsigned from, const unsigned to) {
  unsigned diff = get_move_cost_1(from, to);
  return (1 + diff) * diff / 2;
}

static unsigned calc_diff(const unsigned *const nums, const size_t len, const unsigned num,
                          unsigned (*move_cost)(unsigned, unsigned)) {
  unsigned diff = 0;
  for (size_t i = 0; i < len; i++) {
    diff += move_cost(nums[i], num);
  }
  return diff;
}

static void solution(const char *const input, char *const output, unsigned (*move_cost)(unsigned, unsigned)) {
  size_t npos;
  unsigned *pos = parse_input(input, &npos);

  int start;
  int end;
  get_ranges(pos, npos, &start, &end);

  unsigned mindiff = UINT_MAX;
  for (int p = start; p <= end; p++) {
    unsigned diff = calc_diff(pos, npos, p, move_cost);
    if (diff < mindiff) {
      mindiff = diff;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", mindiff);
  free(pos);
}

static void solution1(const char *const input, char *const output) { solution(input, output, get_move_cost_1); }

static void solution2(const char *const input, char *const output) { solution(input, output, get_move_cost_2); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
