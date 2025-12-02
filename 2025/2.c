#include <aoclib.h>
#include <stdio.h>

// My clever implementation did not work so I'm brute forcing.
// However, know that a closed solution exists.
// https://www.reddit.com/r/adventofcode/comments/1pcbgai/2025_day_2_day_2_should_be_easy_right_closed/

struct range {
  long start, end;
};

static bool parse_range(const char **input, struct range *range) {
  if (**input == '\0') {
    return false;
  }

  range->start = aoc_parse_long(input);
  aoc_expect_char(input, '-');
  range->end = aoc_parse_long(input);

  aoc_skip_space(input);
  if (**input == ',') {
    *input += 1;
    aoc_skip_space(input);
  }

  return true;
}

static int count_digits(long number, int base) {
  int digits = 0;
  while (number > 0) {
    number /= base;
    digits += 1;
  }
  return digits;
}

__attribute__((const)) static long power_of(int base, int exponent) {
  long result = 1;
  for (int i = 0; i < exponent; i++) {
    result *= base;
  }
  return result;
}

__attribute__((const)) static bool is_valid(long number, int subdivisions) {
  int digits = count_digits(number, 10);
  if (digits % subdivisions != 0) {
    return false;
  }

  long factor = power_of(10, digits / subdivisions);
  long section = number % factor;
  number /= factor;
  while (number > 0) {
    if (number % factor != section) {
      return false;
    }
    number /= factor;
  }

  return true;
}

static void solution1(const char *input, char *const output) {
  struct range range;
  long result = 0;
  while (parse_range(&input, &range)) {
    for (long n = range.start; n <= range.end; n++) {
      if (is_valid(n, 2)) {
        result += n;
      }
    }
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

static void solution2(const char *input, char *const output) {
  struct range range;
  long result = 0;
  while (parse_range(&input, &range)) {
    for (long n = range.start; n <= range.end; n++) {
      for (int subdivisions = 2; subdivisions < 15; subdivisions++) {
        if (is_valid(n, subdivisions)) {
          result += n;
          break;
        }
      }
    }
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
