#include <aoclib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

struct parse_input_args {
  struct aoc_point *start;
  struct aoc_point *end;
};

static void parse_input_cb(const char **input, void *vresult, int x, int y, void *vargs) {
  char *result = vresult;
  struct parse_input_args *args = vargs;

  char c = **input;
  *result = c;

  if (c == 'S') {
    args->start->x = x;
    args->start->y = y;
    *result = '.';
  }
  if (c == 'E') {
    args->end->x = x;
    args->end->y = y;
    *result = '.';
  }

  *input += 1;
}

static char *parse_input(const char *input, struct aoc_point *start, struct aoc_point *end, int *width, int *height) {
  struct parse_input_args args;
  args.start = start;
  args.end = end;
  return aoc_parse_grid(&input, parse_input_cb, sizeof(char), height, width, &args);
}

struct state {
  int score;
  struct aoc_point position;
  enum aoc_direction heading;
};

static int state_cmp(const void *v1, const void *v2) {
  const struct state *s1 = v1;
  const struct state *s2 = v2;
  return s2->score - s1->score;
}

static int *find_best_path(const char *map, int width, int height, struct state *starts, int nstarts) {
  int *distances = aoc_malloc(sizeof(*distances) * width * height * 4);
  for (int k = 0; k < 4; k++) {
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        distances[AOC_3D_IDX(i, j, k, width, height)] = INT_MAX;
      }
    }
  }

  struct aoc_heap heap;
  aoc_heap_init(&heap, sizeof(struct state), 64, state_cmp);
  for (int i = 0; i < nstarts; i++) {
    aoc_heap_push(&heap, &starts[i]);
    distances[AOC_3D_IDX(starts[i].position.x, starts[i].position.y, starts[i].heading, width, height)] = 0;
  }

  while (!aoc_heap_empty(&heap)) {
    struct state current;
    aoc_heap_pop(&heap, &current);
    /* DBG("current: %d,%d,%d,%d", current.score, current.position.x, current.position.y, current.heading); */

    if (distances[AOC_3D_IDX(current.position.x, current.position.y, current.heading, width, height)] < current.score) {
      continue;
    }

    for (unsigned i = 0; i < 4; i++) {
      if (i == current.heading) {
        continue;
      }

      int *d = &distances[AOC_3D_IDX(current.position.x, current.position.y, i, width, height)];
      /* DBG("%d", *d); */
      if (*d > current.score + 1000) {
        *d = current.score + 1000;
        struct state next = current;
        next.heading = i;
        next.score = *d;
        aoc_heap_push(&heap, &next);
      }
    }

    struct aoc_point next_position = aoc_point_move(current.position, current.heading);
    int *d = &distances[AOC_3D_IDX(next_position.x, next_position.y, current.heading, width, height)];
    /* DBG("%d", *d); */
    if (aoc_point_in_limits(next_position, width, height) &&
        map[AOC_2D_IDX(next_position.x, next_position.y, width)] != '#' && *d > current.score + 1) {
      *d = current.score + 1;
      struct state next = current;
      next.position = next_position;
      next.score = *d;
      aoc_heap_push(&heap, &next);
    }
  }

  aoc_heap_free(&heap);
  return distances;
}

static void solution1(const char *input, char *const output) {
  struct aoc_point start, end;
  int width, height;
  char *map = parse_input(input, &start, &end, &width, &height);

  struct state starts[1];
  starts[0].position = start;
  starts[0].score = 0;
  starts[0].heading = AOC_DIRECTION_EAST;
  int *distances = find_best_path(map, width, height, starts, 1);

  int best = INT_MAX;
  for (int i = 0; i < 4; i++) {
    int d = distances[AOC_3D_IDX(end.x, end.y, i, width, height)];
    if (d < best) {
      best = d;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", best);
  free(distances);
  free(map);
}

static void solution2(const char *input, char *const output) {
  struct aoc_point start, end;
  int width, height;
  char *map = parse_input(input, &start, &end, &width, &height);

  struct state starts[4];
  starts[0].position = start;
  starts[0].score = 0;
  starts[0].heading = AOC_DIRECTION_EAST;
  int *distances_from_start = find_best_path(map, width, height, starts, 1);

  for (int i = 0; i < 4; i++) {
    starts[i].position = end;
    starts[i].score = 0;
    starts[i].heading = i;
  }
  int *distances_from_end = find_best_path(map, width, height, starts, 4);

  int best = INT_MAX;
  for (int i = 0; i < 4; i++) {
    int d = distances_from_start[AOC_3D_IDX(end.x, end.y, i, width, height)];
    if (d < best) {
      best = d;
    }
  }

  bool *cells = aoc_malloc(sizeof(*cells) * width * height);
  memset(cells, 0, sizeof(*cells) * width * height);
  for (int k = 0; k < 4; k++) {
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        int distance_from_start = distances_from_start[AOC_3D_IDX(i, j, k, width, height)];
        int distance_from_end = distances_from_end[AOC_3D_IDX(i, j, (k + 2) % 4, width, height)];
        if (distance_from_start != INT_MAX && distance_from_end != INT_MAX &&
            distance_from_start + distance_from_end == best) {
          cells[AOC_2D_IDX(i, j, width)] = true;
        }
      }
    }
  }

  int count = 0;
  for (int i = 0; i < width * height; i++) {
    if (cells[i]) {
      count++;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
  free(cells);
  free(distances_from_end);
  free(distances_from_start);
  free(map);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
