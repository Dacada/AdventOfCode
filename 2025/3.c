#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

struct string_slice {
  const char *ptr;
  int len;
};

static bool parse_pack(const char **input, struct string_slice *pack) {
  if (**input == '\0') {
    return false;
  }

  pack->ptr = *input;
  pack->len = 0;
  while (!isspace(**input) && **input != '\0') {
    pack->len += 1;
    *input += 1;
  }

  aoc_skip_space(input);
  return true;
}

__attribute__((const)) static long power_of(int base, int exponent) {
  long result = 1;
  for (int i = 0; i < exponent; i++) {
    result *= base;
  }
  return result;
}

static long best_joltage(struct string_slice pack, int digits) {
  long *best_suffix_up_and_including = malloc(sizeof(*best_suffix_up_and_including) * pack.len * digits);
  long best = 0;
  for (int d = 0; d < digits; d++) {
    for (int i = pack.len - 1 - d; i >= 0; i--) {
      long current = pack.ptr[i] - '0';
      current *= power_of(10, d);
      if (d != 0) {
        ;
        current += best_suffix_up_and_including[AOC_2D_IDX(i + 1, d - 1, digits)];
      }

      if (current > best) {
        best = current;
      }

      best_suffix_up_and_including[AOC_2D_IDX(i, d, digits)] = best;
      // DBG("%d(%d) -> %ld(%ld)", i, d, current, best);
    }
  }
  free(best_suffix_up_and_including);
  return best;
}

static void solution(const char *input, char *const output, int digits) {
  ASSERT(LONG_MAX >= 999999999999, "long max is not big enough :(");
  struct string_slice pack;
  long result = 0;
  while (parse_pack(&input, &pack)) {
    long n = best_joltage(pack, digits);
    DBG("%ld", n);
    result += n;
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

static void solution1(const char *input, char *const output) { solution(input, output, 2); }

static void solution2(const char *input, char *const output) { solution(input, output, 12); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
