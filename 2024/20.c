#include <aoclib.h>
#include <limits.h>
#include <stdio.h>

struct parse_input_args {
  struct aoc_point *start;
  struct aoc_point *end;
};

static void parse_input_cb(const char **input, void *vres, int x, int y, void *vargs) {
  char *res = vres;
  struct parse_input_args *args = vargs;

  *res = **input;
  if (*res == 'S') {
    args->start->x = x;
    args->start->y = y;
    *res = '.';
  }
  if (*res == 'E') {
    args->end->x = x;
    args->end->y = y;
    *res = '.';
  }

  *input += 1;
}

static char *parse_input(const char *input, int *width, int *height, struct aoc_point *start, struct aoc_point *end) {
  struct parse_input_args args;
  args.start = start;
  args.end = end;
  return aoc_parse_grid(&input, parse_input_cb, sizeof(char), height, width, &args);
}

struct state {
  int steps;
  struct aoc_point current;
};

static int cmp_state(const void *v1, const void *v2) {
  const struct state *s1 = v1;
  const struct state *s2 = v2;
  return s2->steps - s1->steps;
}

static int *find_distances(const char *grid, int width, int height, struct aoc_point start, struct aoc_point end) {
  int *distances = aoc_malloc(sizeof(*distances) * width * height);
  for (int i = 0; i < width * height; i++) {
    distances[i] = INT_MAX;
  }

  struct state state;
  state.steps = 0;
  state.current = start;

  struct aoc_heap heap;
  aoc_heap_init(&heap, sizeof(struct state), 32, cmp_state);
  aoc_heap_push(&heap, &state);

  while (!aoc_heap_empty(&heap)) {
    aoc_heap_pop(&heap, &state);

    int *n = &distances[AOC_2D_IDX(state.current.x, state.current.y, width)];
    if (*n <= state.steps) {
      continue;
    }
    *n = state.steps;

    if (state.current.x == end.x && state.current.y == end.y) {
      continue;
    }

    for (int i = 0; i < 4; i++) {
      struct aoc_point next = aoc_point_move(state.current, i);
      if (!aoc_point_in_limits(next, width, height) || grid[AOC_2D_IDX(next.x, next.y, width)] == '#') {
        continue;
      }

      struct state new = state;
      new.current = next;
      new.steps++;
      aoc_heap_push(&heap, &new);
    }
  }

  aoc_heap_free(&heap);
  return distances;
}

static void solution(const char *input, char *const output, int radius) {
  int width, height;
  struct aoc_point start, end;
  char *grid = parse_input(input, &width, &height, &start, &end);

  int count = 0;
  int *distances = find_distances(grid, width, height, start, end);
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < height; i++) {
      int d1 = distances[AOC_2D_IDX(i, j, width)];
      if (d1 == INT_MAX) {
        continue;
      }
      for (int r = 1; r <= radius; r++) {
        for (int di = -r; di <= r; di++) {
          if (i + di < 0 || i + di >= width) {
            continue;
          }
          for (int dj = -r; dj <= r; dj++) {
            if (j + dj < 0 || j + dj >= height) {
              continue;
            }
            if (AOC_ABS(di) + AOC_ABS(dj) != r) {
              continue;
            }
            int d2 = distances[AOC_2D_IDX(i + di, j + dj, width)];
            if (d2 == INT_MAX) {
              continue;
            }

            int saved = d2 - d1 - r;
            if (saved >= 100) {
              DBG("%d,%d -> %d,%d", i, j, i + di, j + dj);
              count++;
            }
          }
        }
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
  free(grid);
  free(distances);
}

static void solution1(const char *input, char *const output) { solution(input, output, 2); }
static void solution2(const char *input, char *const output) { solution(input, output, 20); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
