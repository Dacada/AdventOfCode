#include <aoclib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 71
#define HEIGHT 71

static struct aoc_point *parse_input(const char *input, int *len) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(struct aoc_point), 1024);

  while (*input != '\0') {
    struct aoc_point *p = aoc_dynarr_grow(&arr, 1);
    p->x = aoc_parse_int(&input);
    aoc_expect_char(&input, ',');
    p->y = aoc_parse_int(&input);
    while (*input == '\n') {
      input += 1;
    }
  }

  *len = arr.len;
  return arr.data;
}

struct state {
  int steps;
  struct aoc_point position;
};

static int state_cmp(const void *v1, const void *v2) {
  const struct state *s1 = v1;
  const struct state *s2 = v2;

  return s2->steps - s1->steps;
}

static bool corrupted[HEIGHT][WIDTH];

static bool heap_init = false;
static struct aoc_heap heap;
static int visited[HEIGHT][WIDTH];

static int find_path() {
  if (!heap_init) {
    aoc_heap_init(&heap, sizeof(struct state), 16, state_cmp);
    heap_init = true;
  }

  struct state state;
  state.steps = 0;
  state.position.x = 0;
  state.position.y = 0;
  aoc_heap_push(&heap, &state);

  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      visited[j][i] = INT_MAX;
    }
  }

  int result = -1;
  while (!aoc_heap_empty(&heap)) {
    aoc_heap_pop(&heap, &state);
    if (state.position.x == WIDTH - 1 && state.position.y == HEIGHT - 1) {
      result = state.steps;
      break;
    }

    int *v = &visited[state.position.x][state.position.y];
    if (*v <= state.steps) {
      continue;
    }
    *v = state.steps;

    for (int i = 0; i < 4; i++) {
      struct aoc_point next = aoc_point_move(state.position, i);
      if (!aoc_point_in_limits(next, WIDTH, HEIGHT) || corrupted[next.y][next.x]) {
        continue;
      }

      struct state newstate = state;
      newstate.position = next;
      newstate.steps += 1;
      aoc_heap_push(&heap, &newstate);
    }
  }

  aoc_dynarr_truncate(&heap.arr);
  return result;
}

static void solution1(const char *input, char *const output) {
  int npoints;
  struct aoc_point *points = parse_input(input, &npoints);

  memset(corrupted, 0, sizeof(corrupted));
  for (int i = 0; i < 1024; i++) {
    corrupted[points[i].y][points[i].x] = true;
  }

  int result = find_path();

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
  free(points);
  aoc_heap_free(&heap);
}

struct key {
  struct aoc_point *last;
  bool last_was_blocked;
  struct aoc_point *points;
  int npoints;
};

static int is_path_cut_off(const void *vkey, const void *vcurrent) {
// this is what I like to call a "pro gamer move"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  struct key *key = (struct key *)vkey;
  struct aoc_point *current = (struct aoc_point *)vcurrent;
#pragma GCC diagnostic pop

  key->last = current;

  DBG("%d,%d", current->x, current->y);

  ASSERT(current >= key->points, "oof");
  ASSERT(current < key->points + key->npoints, "oof harder");

  memset(corrupted, 0, sizeof(corrupted));
  for (struct aoc_point *p = key->points; p <= current; p++) {
    corrupted[p->y][p->x] = true;
  }

  int result = find_path();

  if (result > 0) {
    key->last_was_blocked = false;
    return 1;
  } else {
    key->last_was_blocked = true;
    return -1;
  }
}

static void solution2(const char *input, char *const output) {
  int npoints;
  struct aoc_point *points = parse_input(input, &npoints);

  struct key key;
  key.points = points;
  key.npoints = npoints;

  // assumption: bsearch performs an actual binary search, where (if match not
  // found) the last point traversed will be in the boundary between lesser than
  // and greater than and it passes to the comparison function an actual pointer
  // to an element within the array and not, e.g. a copy of it -- for future
  // self: if this breaks, replace it with a linear search and be done with it
  void *res = bsearch(&key, points, npoints, sizeof(*points), is_path_cut_off);
  ASSERT(res == NULL, "expected bsearch to find nothing");

  struct aoc_point *result = key.last;
  if (!key.last_was_blocked) {
    result += 1;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d,%d", result->x, result->y);
  free(points);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
