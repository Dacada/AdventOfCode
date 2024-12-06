#include <aoclib.h>
#include <stdio.h>

static void solution1(const char *const input, char *const output) {
  int counter = 0;
  for (size_t i = 0; input[i] != '\0'; i++) {
    if (input[i] == '(') {
      counter++;
    } else if (input[i] == ')') {
      counter--;
    }
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", counter);
}

static void solution2(const char *const input, char *const output) {
  int counter = 0;
  size_t i;
  for (i = 0; input[i] != '\0'; i++) {
    if (input[i] == '(') {
      counter++;
    } else if (input[i] == ')') {
      counter--;
    }

    if (counter == -1) {
      break;
    }
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", i + 1);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
