#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int parse_input(const char *const input, int *const i) {
  if (input[*i] == '\0') {
    return 0;
  }
  if (input[*i] == '\n') {
    *i += 1;
    return -1;
  }

  int n = 0;
  char c;
  while (isdigit(c = input[*i])) {
    n *= 10;
    n += c - '0';
    *i += 1;
  }
  ASSERT(c == '\n', "parse error");
  *i += 1;
  return n;
}

static int invcmp(const void *const a, const void *const b) {
  const int *aa = a;
  const int *bb = b;
  return *bb - *aa;
}

static void solution(const char *const input, int max[]) {
  int calories;
  int current = 0;
  int i = 0;

  memset(max, 0, sizeof(*max) * 3);
  while ((calories = parse_input(input, &i))) {
    if (calories < 0) {
      if (current > max[2]) {
        max[2] = current;
        qsort(max, 3, sizeof(*max), invcmp);
      }
      current = 0;
    } else {
      current += calories;
    }
  }
}

static void solution1(const char *const input, char *const output) {
  int max[3];
  solution(input, max);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", max[0]);
}

static void solution2(const char *const input, char *const output) {
  int max[3];
  solution(input, max);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", max[0] + max[1] + max[2]);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
