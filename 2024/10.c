#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static void parse_input_cb(const char **input, void *vres, int x, int y, void *vargs) {
  int *res = vres;
  struct aoc_dynarr *arr = vargs;

  *res = **input - '0';
  if (*res == 0) {
    struct aoc_point *p = aoc_dynarr_grow(arr, 1);
    p->x = x;
    p->y = y;
  }

  *input += 1;
}

static void parse_input(const char *input, int **trails, int *width, int *height, struct aoc_dynarr *trailheads) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(struct aoc_point), 4);

  int *res = aoc_parse_grid(&input, parse_input_cb, sizeof(int), height, width, &arr);

  *trails = res;
  *trailheads = arr;
}

static int get_trailhead_score(const int *trails, int height, int width, struct aoc_point trailhead, bool ignore_seen) {
  struct aoc_point stack[64];
  int stacki = 0;
  stack[stacki++] = trailhead;

  bool *seen = NULL;
  if (!ignore_seen) {
    seen = aoc_malloc(sizeof(*seen) * height * width);
    memset(seen, 0, sizeof(*seen) * height * width);
  }

  int score = 0;
  while (stacki > 0) {
    struct aoc_point current = stack[--stacki];
    int n = trails[AOC_2D_IDX(current.x, current.y, width)];
    if (n == 9) {
      if (!ignore_seen && seen[AOC_2D_IDX(current.x, current.y, width)]) {
        continue;
      }
      score++;
      if (!ignore_seen) {
        seen[AOC_2D_IDX(current.x, current.y, width)] = true;
      }
      continue;
    }

    for (int i = 0; i < 4; i++) {
      struct aoc_point next = current;
      switch (i) {
      case 0:
        next.x++;
        break;
      case 1:
        next.x--;
        break;
      case 2:
        next.y++;
        break;
      case 3:
        next.y--;
        break;
      default:
        FAIL("logic error");
      }

      if (next.x < 0 || next.x >= width || next.y < 0 || next.y >= height) {
        continue;
      }

      int m = trails[AOC_2D_IDX(next.x, next.y, width)];
      if (m != n + 1) {
        continue;
      }

      stack[stacki++] = next;
      if (stacki == 64) {
        FAIL("stack full");
      }
    }
  }

  DBG("score for trailhead at %d,%d is %d", trailhead.x, trailhead.y, score);

  free(seen);
  return score;
}

static void solution(const char *input, char *const output, bool ignore_seen) {
  int *trails;
  int height;
  int width;
  struct aoc_dynarr trailheads;
  parse_input(input, &trails, &height, &width, &trailheads);

  int total = 0;
  for (int i = 0; i < trailheads.len; i++) {
    struct aoc_point p = AOC_DYNARR_IDX(trailheads, i, struct aoc_point);
    total += get_trailhead_score(trails, height, width, p, ignore_seen);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(trails);
  aoc_dynarr_free(&trailheads);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
