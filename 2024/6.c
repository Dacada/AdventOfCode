#include <aoclib.h>
#include <stdio.h>
#include <string.h>

enum cell {
  CELL_FREE,
  CELL_OBSTACLE,
  CELL_VISITED,
};

enum direction {
  DIR_NORTH,
  DIR_EAST,
  DIR_SOUTH,
  DIR_WEST,
};

struct point {
  int x;
  int y;
};

static void parse_cb(const char **input, void *cell, int x, int y, void *args) {
  struct point *point = args;

  enum cell c;
  if (**input == '.') {
    c = CELL_FREE;
  } else if (**input == '#') {
    c = CELL_OBSTACLE;
  } else if (**input == '^') {
    c = CELL_FREE;
    point->x = x;
    point->y = y;
  } else {
    FAIL("invalid cell %c", **input);
  }

  *((enum cell *)cell) = c;
  *input += 1;
}

static bool advance(enum cell *cells, struct aoc_dynarr *arr,
                    struct point *location, enum direction *direction,
                    int ncols, int nrows) {
  int limit;
  int *coord;
  int inc;
  switch (*direction) {
  case DIR_NORTH:
    limit = 0;
    coord = &location->y;
    inc = -1;
    break;
  case DIR_EAST:
    limit = ncols - 1;
    coord = &location->x;
    inc = 1;
    break;
  case DIR_SOUTH:
    limit = nrows - 1;
    coord = &location->y;
    inc = 1;
    break;
  case DIR_WEST:
    limit = 0;
    coord = &location->x;
    inc = -1;
    break;
  default:
    FAIL("invalid direction %d", *direction);
  }

  for (;;) {
    enum cell *c = &cells[location->y * ncols + location->x];
    if (*c == CELL_OBSTACLE) {
      *coord -= inc;
      *direction = (*direction + 1) % 4;
      return true;
    }

    if (*c == CELL_FREE) {
      if (arr != NULL) {
        struct point *p = aoc_dynarr_grow(arr, 1);
        *p = *location;
      }
      *c = CELL_VISITED;
    }

    if (*coord == limit) {
      return false;
    } else {
      *coord += inc;
    }
  }
}

static struct aoc_dynarr get_guard_locations(const enum cell *cells,
                                             struct point start, int ncols,
                                             int nrows) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(struct point), 8);

  enum cell *cells_copy = malloc(ncols * nrows * sizeof(*cells_copy));
  memcpy(cells_copy, cells, ncols * nrows * sizeof(*cells));

  struct point location = start;
  enum direction dir = DIR_NORTH;

  while (advance(cells_copy, &arr, &location, &dir, ncols, nrows))
    ;

  free(cells_copy);
  return arr;
}

static bool guard_would_loop(const enum cell *cells, struct point start,
                             int ncols, int nrows) {
  enum cell *cells_copy = malloc(ncols * nrows * sizeof(*cells_copy));
  memcpy(cells_copy, cells, ncols * nrows * sizeof(*cells));

  struct point location = start;
  enum direction dir = DIR_NORTH;

  bool *memory[4] = {
      malloc(ncols * nrows * sizeof(bool)),
      malloc(ncols * nrows * sizeof(bool)),
      malloc(ncols * nrows * sizeof(bool)),
      malloc(ncols * nrows * sizeof(bool)),
  };
  for (int i = 0; i < 4; i++) {
    memset(memory[i], 0, ncols * nrows * sizeof(bool));
  }
  memory[dir][location.y * ncols + location.x] = true;

  bool looping = false;
  while (advance(cells_copy, NULL, &location, &dir, ncols, nrows)) {
    bool *mem = &memory[dir][location.y * ncols + location.x];
    if (*mem) {
      looping = true;
      break;
    }
    *mem = true;
  }

  free(cells_copy);
  for (int i = 0; i < 4; i++) {
    free(memory[i]);
  }
  return looping;
}

static void solution1(const char *const input, char *const output) {
  struct point start;
  int nrows, ncols;
  enum cell *cells = aoc_parse_grid(input, parse_cb, sizeof(enum cell), &nrows,
                                    &ncols, &start);

  struct aoc_dynarr arr = get_guard_locations(cells, start, ncols, nrows);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", arr.len);
  free(cells);
  aoc_dynarr_free(&arr);
}

static void solution2(const char *const input, char *const output) {
  struct point start;
  int nrows, ncols;
  enum cell *cells = aoc_parse_grid(input, parse_cb, sizeof(enum cell), &nrows,
                                    &ncols, &start);

  struct aoc_dynarr arr = get_guard_locations(cells, start, ncols, nrows);

  int result = 0;
  struct point *locations = arr.data;
  for (int i = 0; i < arr.len; i++) {
    struct point p = locations[i];
    ASSERT(cells[p.y * ncols + p.x] == CELL_FREE,
           "guard was in a nonfree cell?");
    cells[p.y * ncols + p.x] = CELL_OBSTACLE;
    if (guard_would_loop(cells, start, ncols, nrows)) {
      result++;
    }
    cells[p.y * ncols + p.x] = CELL_FREE;
    DBG("%d/%d", i, arr.len);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
  free(cells);
  aoc_dynarr_free(&arr);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
