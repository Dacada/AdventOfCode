#include <aoclib.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static bool parse_line(const char **input, int *const inc) {
  if (**input == '\0') {
    return false;
  }
  aoc_skip_space(input);

  int sign;
  if (**input == 'L') {
    sign = -1;
  } else if (**input == 'R') {
    sign = 1;
  } else {
    FAIL("input parse error");
  }
  *input += 1;

  int num = aoc_parse_int(input);
  aoc_skip_space(input);

  *inc = sign * num;

  return true;
}

static int true_floor_div(int x, int y) {
  int q = x / y;
  int r = x % y;

  if (r != 0 && ((x < 0) ^ (y < 0))) {
    q -= 1;
  }

  return q;
}

static void solution(const char *input, char *const output, bool click_method) {
  int curr = 50;
  int inc;
  int res = 0;

  while (parse_line(&input, &inc)) {
    DBG("%d + %d", curr, inc);

    int next = curr + inc;
    DBG("next = %d", next);

    if (click_method) {
      int start = MIN(curr, next);
      int end = MAX(curr, next);
      int crossings = true_floor_div(end - 1, 100) - true_floor_div(start, 100);
      DBG("crossings = %d", crossings);
      res += crossings;
    }

    curr = aoc_modulo_int(next, 100);
    if (curr == 0) {
      res += 1;
      DBG("result is 0");
    }

    DBG("current total: %d", res);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
