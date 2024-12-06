#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>

#define MODULO 20201227U

struct problem {
  unsigned long card;
  unsigned long door;
};

static unsigned long parse_int(const char **const input) {
  unsigned long n = 0;
  char c;
  while (isdigit(c = **input)) {
    n = n * 10 + c - '0';
    *input += 1;
  }
  return n;
}

__attribute__((pure)) static struct problem parse(const char *input) {
  struct problem problem;
  problem.card = parse_int(&input);
  ASSERT(*input == '\n', "parse error");
  input++;
  problem.door = parse_int(&input);
  return problem;
}

__attribute__((const)) static unsigned long fast_modular_power(unsigned long base, unsigned long exponent,
                                                               unsigned long modulo) {
  unsigned long result = 1;
  if (exponent & 1U) {
    result = base;
  }
  while (exponent) {
    exponent >>= 1;
    base = (base * base) % modulo;
    if (exponent & 1U) {
      result = (result * base) % modulo;
    }
  }
  return result;
}

static void solution1(const char *const input, char *const output) {
  struct problem problem = parse(input);

  unsigned long exp;
  for (exp = 1;; exp++) {
    if (fast_modular_power(7U, exp, MODULO) == problem.card) {
      break;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", fast_modular_power(problem.door, exp, MODULO));
}

static void solution2(const char *const input, char *const output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "Merry Christmas :D");
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
