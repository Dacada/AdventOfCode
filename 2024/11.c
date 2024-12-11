#include <aoclib.h>
#include <stdio.h>

static long *parse_input(const char *input, long *nstones) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(long), 8);

  while (*input != '\0') {
    long *n = aoc_dynarr_grow(&arr, 1);
    *n = aoc_parse_long(&input);
    aoc_skip_space(&input);
  }

  *nstones = arr.len;
  return arr.data;
}

static bool split(long num, long *left, long *right) {
  long orig = num;
  long power_of_ten = 1;
  while (num > 0) {
    num /= 10;
    if (num == 0) {
      return false;
    }
    num /= 10;
    power_of_ten *= 10;
  }
  *left = orig / power_of_ten;
  *right = orig % power_of_ten;
  return true;
}

struct res {
  long stone;
  long result;
};
static struct aoc_dynarr memo[76][10007];
static bool memo_get(long stone, long steps, long *result) {
  struct aoc_dynarr arr = memo[steps][stone % 10007];
  for (int i = 0; i < arr.len; i++) {
    struct res r = AOC_DYNARR_IDX(arr, i, struct res);
    if (r.stone == stone) {
      *result = r.result;
      return true;
    }
  }
  return false;
}
static void memo_set(long stone, long steps, long result) {
  struct aoc_dynarr *arr = &memo[steps][stone % 10007];
  if (arr->size == 0) {
    aoc_dynarr_init(arr, sizeof(struct res), 16);
  }
  struct res *r = aoc_dynarr_grow(arr, 1);
  r->stone = stone;
  r->result = result;
}
#ifdef DEBUG
long memo_hit = 0;
long memo_miss = 0;
#endif
static long get_stone_count_after_steps(long stone, long steps) {
  if (steps == 0) {
    return 1;
  }

  long res;
  if (memo_get(stone, steps, &res)) {
#ifdef DEBUG
    memo_hit++;
#endif
    return res;
  }

  if (stone == 0) {
    res = get_stone_count_after_steps(1, steps - 1);
  } else {
    long left, right;
    if (split(stone, &left, &right)) {
      res = get_stone_count_after_steps(left, steps - 1) + get_stone_count_after_steps(right, steps - 1);
    } else {
      res = get_stone_count_after_steps(stone * 2024, steps - 1);
    }
  }

  memo_set(stone, steps, res);
#ifdef DEBUG
  memo_miss++;
#endif
  return res;
}

static void solution(const char *input, char *const output, long steps) {
  long nstones;
  long *stones = parse_input(input, &nstones);

  long total = 0;
  for (long i = 0; i < nstones; i++) {
    total += get_stone_count_after_steps(stones[i], steps);
  }

  DBG("memo: %ld/%ld", memo_hit, memo_hit + memo_miss);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
  free(stones);
}

static void solution1(const char *input, char *const output) { solution(input, output, 25); }

static void solution2(const char *input, char *const output) { solution(input, output, 75); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
