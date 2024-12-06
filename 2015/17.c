#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define LITERS 150
#define NCONTAINERS 20

static void parse(const char *input, unsigned int *const containers) {
  for (size_t i = 0; i < NCONTAINERS; i++) {
    containers[i] = 0;

    while (*input >= '0' && *input <= '9') {
      containers[i] = containers[i] * 10 + (*input) - 0x30;
      input++;
    }

    ASSERT(*input == '\n', "Did not parse full line");
    input++;
  }
}

struct args_t {
  unsigned int total;
  size_t len;
};

static void fits_exactly(int *const containers_int, void *args_void) {
  struct args_t *args = args_void;
  unsigned int *containers = (unsigned int *)containers_int;

  unsigned int sum = 0;
  for (size_t i = 0; i < args->len; i++) {
    sum += containers[i];
  }

  if (sum == LITERS) {
    args->total += 1;
  }
}

static unsigned int solve1(const unsigned int *const containers) {
  struct args_t args;
  args.total = 0;

  for (size_t ncomb = 1; ncomb <= NCONTAINERS; ncomb++) {
    args.len = ncomb;
    aoc_combinations((const int *const)containers, NCONTAINERS, ncomb, fits_exactly, &args);
  }

  return args.total;
}

static unsigned int solve2(const unsigned int *const containers) {
  struct args_t args;
  args.total = 0;

  for (size_t ncomb = 1; ncomb <= NCONTAINERS; ncomb++) {
    args.len = ncomb;
    aoc_combinations((const int *const)containers, NCONTAINERS, ncomb, fits_exactly, &args);
    if (args.total != 0) {
      return args.total;
    }
  }

  FAIL("Did not find any matching containers");
}

static void solution1(const char *const input, char *const output) {
  unsigned int containers[NCONTAINERS];
  parse(input, containers);
  unsigned int combinations = solve1(containers);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", combinations);
}

static void solution2(const char *const input, char *const output) {
  unsigned int containers[NCONTAINERS];
  parse(input, containers);
  unsigned int combinations = solve2(containers);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", combinations);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
