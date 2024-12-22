#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static unsigned long *parse_input(const char *input, int *len) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(unsigned long), 64);
  while (*input != '\0') {
    unsigned long *n = aoc_dynarr_grow(&arr, 1);
    *n = (unsigned long)aoc_parse_long(&input);
    while (*input == '\n') {
      input++;
    }
  }
  *len = arr.len;
  return arr.data;
}

static unsigned long advance(unsigned long n) {
  n ^= n * 64;
  n %= 16777216;
  n ^= n / 32;
  n %= 16777216;
  n ^= n * 2048;
  n %= 16777216;
  return n;
}

static void solution1(const char *input, char *const output) {
  int nnums;
  unsigned long *nums = parse_input(input, &nnums);

  unsigned long total = 0;
  for (int i = 0; i < nnums; i++) {
    unsigned long n = nums[i];
    for (int j = 0; j < 2000; j++) {
      n = advance(n);
    }
    total += n;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", total);
  free(nums);
}

static void solution2(const char *input, char *const output) {
  int nnums;
  unsigned long *nums = parse_input(input, &nnums);

  static int total_sums[19][19][19][19];
  static bool local_seens[19][19][19][19];

  int max = 0;
  for (int i = 0; i < nnums; i++) {
    unsigned long prev = nums[i];
    DBG("%ld", prev);
    int diffs[4];
    for (int j = 0; j < 2000; j++) {
      unsigned long curr = advance(prev);
      int c = curr % 10;
      int p = prev % 10;
      prev = curr;

      int d = c - p;
      diffs[3] = diffs[2];
      diffs[2] = diffs[1];
      diffs[1] = diffs[0];
      diffs[0] = d + 9;
      if (j >= 3) {
        bool *local_seen = &local_seens[diffs[0]][diffs[1]][diffs[2]][diffs[3]];
        int *total_sum = &total_sums[diffs[0]][diffs[1]][diffs[2]][diffs[3]];

        if (*local_seen) {
          continue;
        }
        *local_seen = true;

        *total_sum += c;
        if (*total_sum > max) {
          max = *total_sum;
          DBG("%d,%d,%d,%d -> %d", diffs[0] - 9, diffs[1] - 9, diffs[2] - 9, diffs[3] - 9, c);
          DBG("local_seen: %d | total_sum: %d", *local_seen, *total_sum);
        }
      }
    }
    DBG("reset local seen");
    memset(local_seens, 0, sizeof(local_seens));
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", max);
  free(nums);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
