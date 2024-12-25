#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define NPOINTS 8

static void parse_input_callback(const char **input, void *vres, int x, int y, void *vargs) {
  struct aoc_point *points = vargs;
  char *res = vres;

  if (isdigit(**input)) {
    struct aoc_point *p = &points[**input - '0'];
    p->x = x;
    p->y = y;
    *res = '.';
  } else {
    *res = **input;
  }

  *input += 1;
}

struct permute_args {
  int *adjacency;
  int result;
  bool go_back;
};

static void permute_cb(int *array, void *vargs) {
  struct permute_args *args = vargs;

  int cost = 0;
  int prev = 0;
  for (int i = 0; i < NPOINTS - 1; i++) {
    int distance = args->adjacency[prev * NPOINTS + array[i]];
    ASSERT(distance != INT_MAX, "missing distance between %d and %d", prev, array[i]);
    cost += distance;
    if (cost > args->result) {
      return;
    }
    prev = array[i];
  }

  if (cost < args->result) {
    if (args->go_back) {
      cost += args->adjacency[0 * NPOINTS + prev];
    }
    if (cost < args->result) {
      args->result = cost;
    }
  }
}

static int djkstra(const char *grid, int width, int height, struct aoc_point start, struct aoc_point end) {
  struct state {
    int negcost;
    struct aoc_point curr;
  };

  int *seen = aoc_malloc(width * height * sizeof(*seen));
  for (int i = 0; i < width * height; i++) {
    seen[i] = INT_MAX;
  }

  struct aoc_heap heap;
  aoc_heap_init(&heap, sizeof(struct state), 16, aoc_cmp_int);

  struct state curr;
  curr.negcost = 0;
  curr.curr = start;
  aoc_heap_push(&heap, &curr);

  int res = INT_MAX;
  while (!aoc_heap_empty(&heap)) {
    aoc_heap_pop(&heap, &curr);

    if (curr.curr.x == end.x && curr.curr.y == end.y) {
      res = -curr.negcost;
      break;
    }

    int *d = &seen[AOC_2D_IDX(curr.curr.x, curr.curr.y, width)];
    if (-curr.negcost >= *d) {
      continue;
    }
    *d = -curr.negcost;

    for (int i = 0; i < 4; i++) {
      struct aoc_point next = aoc_point_move(curr.curr, i);
      if (!aoc_point_in_limits(next, width, height)) {
        continue;
      }
      if (grid[AOC_2D_IDX(next.x, next.y, width)] == '#') {
        continue;
      }

      struct state next_state;
      next_state.curr = next;
      next_state.negcost = curr.negcost - 1;
      aoc_heap_push(&heap, &next_state);
    }
  }

  aoc_heap_free(&heap);
  free(seen);
  return res;
}

static void solution(const char *input, char *const output, bool go_back) {
  int height, width;
  struct aoc_point points[NPOINTS];
  char *grid = aoc_parse_grid(&input, parse_input_callback, sizeof(char), &height, &width, points);

  int adjacency[NPOINTS * NPOINTS];
  for (int i = 0; i < NPOINTS * NPOINTS; i++) {
    adjacency[i] = INT_MAX;
  }
  for (int i = 0; i < NPOINTS; i++) {
    adjacency[i * NPOINTS + i] = 0;
    for (int j = i + 1; j < NPOINTS; j++) {
      int distance = djkstra(grid, width, height, points[i], points[j]);
      ASSERT(distance != INT_MAX, "no path between %d and %d", i, j);
      adjacency[i * NPOINTS + j] = distance;
      adjacency[j * NPOINTS + i] = distance;
    }
  }

  struct permute_args args;
  args.go_back = go_back;
  args.adjacency = adjacency;
  args.result = INT_MAX;

  int array[NPOINTS - 1];
  for (int i = 0; i < NPOINTS - 1; i++) {
    array[i] = i + 1;
  }
  aoc_permute(array, NPOINTS - 1, permute_cb, &args);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", args.result);
  free(grid);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
