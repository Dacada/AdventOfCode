#include <aoclib.h>
#include <limits.h>
#include <stdio.h>

static void solution(const char *const input, char *const output, unsigned max_deliveries,
                     unsigned presents_per_house) {
  unsigned goal = atol(input);
  unsigned limit = 2 << 19;
  unsigned *houses = malloc(limit * sizeof(unsigned));

  for (unsigned i = 0; i < limit; i++) {
    houses[i] = 0;
  }

  for (unsigned elf = 1; elf < limit; elf++) {
    unsigned deliveries = 0;
    for (unsigned house = elf; house < limit && deliveries < max_deliveries; house += elf) {
      houses[house] += elf * presents_per_house;
      deliveries++;
    }
  }

  unsigned result = 0;
  for (unsigned house = 0; house < limit; house++) {
    if (houses[house] >= goal) {
      result = house;
      break;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
  free(houses);
  return;
}

static void solution1(const char *const input, char *const output) { solution(input, output, UINT_MAX, 10); }

static void solution2(const char *const input, char *const output) { solution(input, output, 50, 11); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
