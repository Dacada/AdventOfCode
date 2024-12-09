#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

static int *parse_input(const char *input, int *len) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(int), 128);
  for (;;) {
    char c = *input;
    input += 1;
    if (c == '\0') {
      break;
    }
    if (isspace(c)) {
      continue;
    }
    ASSERT(isdigit(c), "parse error");
    int *n = aoc_dynarr_grow(&arr, 1);
    *n = c - '0';
  }
  *len = arr.len;
  return arr.data;
}

static void solution1(const char *input, char *const output) {
  int len;
  int *nums = parse_input(input, &len);

  // good luck understanding what this does
  // it worked first try too, after writing it

  long total = 0;
  int p = 0;
  for (int i = 0; i < len; i++) {
    long id;
    long n;
    if (i % 2 == 0) {
      id = i / 2;
      n = nums[i];
    } else {
      id = len / 2;
      int a = nums[i];
      int b = nums[len - 1];
      if (a > b) {
        len -= 2;
        nums[i] -= b;
        i -= 1;
        n = b;
      } else {
        nums[len - 1] -= a;
        if (nums[len - 1] == 0) {
          len -= 2;
        }
        n = a;
      }
    }
    total += id * (n * p + (n * (n - 1)) / 2);
    p += n;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
  free(nums);
}

static void solution2(const char *input, char *const output) {
  int len;
  int *nums = parse_input(input, &len);

  int *offsets = aoc_malloc(sizeof(*offsets) * len);
  offsets[0] = 0;
  for (int i = 0; i < len - 1; i++) {
    offsets[i + 1] = offsets[i] + nums[i];
  }

  long total = 0;
  for (int i = len - 1; i >= 0; i -= 2) {
    long id = i / 2;
    int size = nums[i];
    int offset = offsets[i];
    for (int j = 1; j < i; j += 2) { // could memoize this, but too complicated
      int hole = nums[j];
      if (hole >= size) {
        offset = offsets[j];
        nums[j] = hole - size;
        offsets[j] += size;
        break;
      }
    }
    total += id * (size * offset + (size * (size - 1)) / 2);
    DBG("File with id %ld goes on offset %d for %d blocks", id, offset, size);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
  free(nums);
  free(offsets);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
