#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void skip_blank(const char **input) {
  while (isblank(**input)) {
    *input += 1;
  }
}

static void assert_str(const char **input, const char *exp) {
  int n = strlen(exp);
  ASSERT(strncmp(*input, exp, n) == 0, "parse error '%s' '%s'", *input, exp);
  *input += n;
}

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error '%d' '%c'", **input, **input);

  int n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static int parse_input(const char *input, int **times, int **distances) {
  int len = 0;
  int cap = 2;
  int *list = malloc(sizeof(*list) * cap);

  assert_str(&input, "Time:");
  while (*input != '\n') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    skip_blank(&input);
    list[len++] = parse_int(&input);
  }
  input++;
  *times = list;

  *distances = malloc(sizeof(**distances) * len);
  assert_str(&input, "Distance:");
  skip_blank(&input);
  for (int i = 0; i < len; i++) {
    (*distances)[i] = parse_int(&input);
    skip_blank(&input);
  }

  return len;
}

__attribute__((const)) static long isqrt(long n) {
  long x = n;
  long y = (x + 1) / 2;
  while (y < x) {
    x = y;
    y = (x + n / x) / 2;
  }
  return x;
}

__attribute__((const)) static long number_of_ways(long x, long t) {
  /* solve (t-i)*i=x for t */
  long i1 = (t + isqrt(t * t - 4 * x)) / 2;
  long i2 = (t - isqrt(t * t - 4 * x)) / 2;

  /* adjust solutions to be within valid range */
  if ((t - i1) * i1 <= x) {
    i1 -= 1;
  }
  if ((t - i2) * i2 <= x) {
    i2 += 1;
  }

  /* number of integers between both solutions */
  return i1 - i2 + 1;
}

static void solution1(const char *const input, char *const output) {
  int *times;
  int *distances;
  int len = parse_input(input, &times, &distances);

  int result = 1;
  for (int i = 0; i < len; i++) {
    int n = number_of_ways(distances[i], times[i]);
    result *= n;
  }

  free(times);
  free(distances);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static long parse_long_bad_kerning(const char **input) {
  long n = 0;
  skip_blank(input);
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
    skip_blank(input);
  }
  return n;
}

static void parse_input_2(const char *input, long *time, long *distance) {
  assert_str(&input, "Time:");
  *time = parse_long_bad_kerning(&input);
  ASSERT(*input == '\n', "parse error");
  input++;

  assert_str(&input, "Distance:");
  *distance = parse_long_bad_kerning(&input);
}

static void solution2(const char *const input, char *const output) {
  long time, distance;
  parse_input_2(input, &time, &distance);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", number_of_ways(distance, time));
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
