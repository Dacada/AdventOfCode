#include <aoclib.h>
#include <stdio.h>

static void solution1(char *input, char *output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

static void solution2(char *input, char *output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
