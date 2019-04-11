#include <aoclib.h>
#include <stdio.h>

static unsigned long get_wrapping_size(const int *const dims) {
  int a = dims[0]*dims[1];
  int b = dims[1]*dims[2];
  int c = dims[0]*dims[2];
  int min = a < b ? a: b; min = min < c ? min : c;
  return 2*a + 2*b + 2*c + min;
}

static unsigned long get_ribbon_size(const int *const dims) {
  int a = dims[0]+dims[0]+dims[1]+dims[1];
  int b = dims[0]+dims[0]+dims[2]+dims[2];
  int c = dims[1]+dims[1]+dims[2]+dims[2];
  int min = a;

  if (b < min) {
    min = b;
  }
  if (c < min) {
    min = c;
  }

  return min + dims[0]*dims[1]*dims[2];
}

static void solution(const char *const input, char *const output, unsigned long (*const get_size)(const int *const)) {
  unsigned long total = 0;
  for (size_t i = 0; input[i] != '\0'; i++) {
    int dims[] = { 0, 0, 0 };
    int dim_i = 0;
    char c;
    for (; (c = input[i]) != '\n'; i++) {
      if (c == 'x') {
	dim_i++;
      } else {
	dims[dim_i] = dims[dim_i]*10 + (c - 0x30);
      }
    }

    total += get_size(dims);
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", total);
}

static void solution1(const char *const input, char *const output) {
  solution(input, output, get_wrapping_size);
}

static void solution2(const char *const input, char *const output) {
  solution(input, output, get_ribbon_size);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
