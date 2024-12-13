#include <aoclib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct group {
  int area;
  int sides;
};

static int get_fencing_price(struct aoc_point start, const char *plots, struct group **searched,
                             struct aoc_dynarr *to_search, int height, int width) {
  char name = plots[AOC_2D_IDX(start.x, start.y, width)];

  struct aoc_dynarr stack;
  aoc_dynarr_init(&stack, sizeof(struct aoc_point), 16);
  struct aoc_point *p = aoc_dynarr_grow(&stack, 1);
  *p = start;

  int area = 0;
  int perimeter = 0;

  struct group *current_group = aoc_malloc(sizeof(*current_group));

  while (stack.len > 0) {
    p = aoc_dynarr_shrink(&stack, 1);
    struct aoc_point current = *p;
    if (searched[AOC_2D_IDX(current.x, current.y, width)] != NULL) {
      continue;
    }
    area++;
    searched[AOC_2D_IDX(current.x, current.y, width)] = current_group;

    for (int i = 0; i < 4; i++) {
      struct aoc_point next = aoc_point_move(current, i);
      if (!aoc_point_in_limits(next, width, height)) {
        perimeter++;
        continue;
      }
      if (plots[AOC_2D_IDX(next.x, next.y, width)] != name) {
        perimeter++;
        p = aoc_dynarr_grow(to_search, 1);
        *p = next;
        continue;
      }
      p = aoc_dynarr_grow(&stack, 1);
      *p = next;
    }
  }

  aoc_dynarr_free(&stack);
  current_group->area = area;
  return area * perimeter;
}

static int cmp_ptr(const void *v1, const void *v2) {
  const void *const *p1 = v1;
  const void *const *p2 = v2;
  uintptr_t addr1 = (uintptr_t)*p1;
  uintptr_t addr2 = (uintptr_t)*p2;
  return (addr1 > addr2) - (addr1 < addr2);
}

static void solution(const char *input, char *const output, bool use_sides) {
  int height, width;
  char *plots = aoc_parse_grid_chars(&input, &height, &width);

  struct aoc_dynarr to_search;
  aoc_dynarr_init(&to_search, sizeof(struct aoc_point), 16);
  struct aoc_point *p = aoc_dynarr_grow(&to_search, 1);
  p->x = 0;
  p->y = 0;

  struct group **searched = aoc_malloc(sizeof(struct group *) * height * width);
  memset(searched, 0, sizeof(struct group *) * height * width);

  int total = 0;
  for (int i = 0; i < to_search.len; i++) {
    struct aoc_point start = AOC_DYNARR_IDX(to_search, i, struct aoc_point);
    if (searched[AOC_2D_IDX(start.x, start.y, width)] != NULL) {
      continue;
    }

    int n = get_fencing_price(start, plots, searched, &to_search, height, width);
    if (!use_sides) {
      total += n;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(plots);
  aoc_dynarr_free(&to_search);

  qsort(searched, width * height, sizeof(struct group *), cmp_ptr);
  void *curr = NULL;
  for (int i = 0; i < width * height; i++) {
    void *ptr = searched[i];
    if (ptr != curr) {
      curr = ptr;
      free(ptr);
    }
  }

  free(searched);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
