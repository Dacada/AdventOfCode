#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define HEXDIM 100

enum direction {
  DIR_EAST,
  DIR_SOUTHEAST,
  DIR_SOUTHWEST,
  DIR_WEST,
  DIR_NORTHWEST,
  DIR_NORTHEAST,

  DIR_LAST
};

// https://www.redblobgames.com/grids/hexagons/
// Axial Coordinates
// Big help
struct point {
  int q, r;
};

static void point_add(struct point *const p1, const struct point p2) {
  p1->q += p2.q;
  p1->r += p2.r;
}

__attribute__((pure)) static struct point direction(const enum direction direction) {
  struct point p;
  switch (direction) {
  case DIR_EAST:
    p.q = +1;
    p.r = 0;
    break;
  case DIR_SOUTHEAST:
    p.q = 0;
    p.r = +1;
    break;
  case DIR_SOUTHWEST:
    p.q = -1;
    p.r = +1;
    break;
  case DIR_WEST:
    p.q = -1;
    p.r = 0;
    break;
  case DIR_NORTHWEST:
    p.q = 0;
    p.r = -1;
    break;
  case DIR_NORTHEAST:
    p.q = +1;
    p.r = -1;
    break;
  case DIR_LAST:
  default:
    FAIL("invalid direction");
    break;
  }
  return p;
}

static struct point follow_path(const enum direction *const path) {
  struct point p = {
      .q = 0,
      .r = 0,
  };

  for (size_t i = 0;; i++) {
    if (path[i] == DIR_LAST) {
      break;
    }
    struct point mp = direction(path[i]);
    point_add(&p, mp);
  }

  return p;
}

static size_t idx_grid(struct point p, const size_t side) {
  unsigned n = 0;
  if (p.q < 0) {
    n |= 1 << 0;
    p.q = -p.q;
  }
  if (p.r < 0) {
    n |= 1 << 1;
    p.r = -p.r;
  }
  return n * side * side + p.r * side + p.q;
}

static enum direction parse_dir(const char **const input) {
  if (**input == 'e') {
    *input += 1;
    return DIR_EAST;
  } else if (**input == 'w') {
    *input += 1;
    return DIR_WEST;
  } else if (**input == 'n') {
    *input += 1;
    if (**input == 'w') {
      *input += 1;
      return DIR_NORTHWEST;
    } else if (**input == 'e') {
      *input += 1;
      return DIR_NORTHEAST;
    } else {
      FAIL("parse error");
    }
  } else if (**input == 's') {
    *input += 1;
    if (**input == 'w') {
      *input += 1;
      return DIR_SOUTHWEST;
    } else if (**input == 'e') {
      *input += 1;
      return DIR_SOUTHEAST;
    } else {
      FAIL("parse error");
    }
  } else {
    FAIL("parse error");
  }
}

static enum direction *parse_line(const char **const input, size_t *const length) {
  size_t capacity = 8;
  enum direction *list = malloc(capacity * sizeof *list);
  size_t len = 0;

  for (; **input != '\n' && **input != '\0';) {
    list[len++] = parse_dir(input);

    if (len >= capacity) {
      capacity *= 2;
      list = realloc(list, capacity * sizeof *list);
    }
  }

  *length = len;
  list[len] = DIR_LAST;
  return list;
}

static enum direction **parse(const char *input, size_t *const farthest_point) {
  size_t capacity = 8;
  enum direction **list = malloc(capacity * sizeof *list);
  size_t len = 0;

  *farthest_point = 0;

  for (; *input != '\0'; input++) {
    size_t length;
    list[len++] = parse_line(&input, &length);
    if (length > *farthest_point) {
      *farthest_point = length;
    }

    if (len >= capacity) {
      capacity *= 2;
      list = realloc(list, capacity * sizeof *list);
    }

    if (*input == '\0') {
      break;
    }
    ASSERT(*input == '\n', "parse error");
  }

  list[len] = NULL;
  return list;
}

static unsigned count_black_neighbors(const struct point p, const bool *const grid, const size_t side) {
  unsigned count = 0;
  for (enum direction d = 0; d < DIR_LAST; d++) {
    struct point q = p;
    struct point mp = direction(d);
    point_add(&q, mp);

    size_t idx = idx_grid(q, side);
    bool black = grid[idx];

    if (black) {
      count++;
    }
  }
  return count;
}

static void solution(const char *const input, char *const output, bool part2) {
  size_t farthest_point;
  enum direction **directions = parse(input, &farthest_point);
  if (part2) {
    farthest_point += 50;
  }

#ifdef DEBUG
  for (size_t i = 0;; i++) {
    if (directions[i] == NULL) {
      break;
    }

    for (size_t j = 0;; j++) {
      if (directions[i][j] == DIR_LAST) {
        break;
      }

      switch (directions[i][j]) {
      case DIR_EAST:
        fprintf(stderr, "e");
        break;
      case DIR_SOUTHEAST:
        fprintf(stderr, "se");
        break;
      case DIR_SOUTHWEST:
        fprintf(stderr, "sw");
        break;
      case DIR_WEST:
        fprintf(stderr, "w");
        break;
      case DIR_NORTHWEST:
        fprintf(stderr, "nw");
        break;
      case DIR_NORTHEAST:
        fprintf(stderr, "ne");
        break;
      case DIR_LAST:
      default:
        FAIL("impossible");
        break;
      }
    }

    fprintf(stderr, "\n");
  }
#endif

  size_t size = 4 * farthest_point * farthest_point * sizeof(bool);
  bool *grid = malloc(size);
  memset(grid, 0, size);

  for (size_t i = 0;; i++) {
    enum direction *path = directions[i];
    if (path == NULL) {
      break;
    }

    struct point p = follow_path(path);
    size_t idx = idx_grid(p, farthest_point);
    grid[idx] = !grid[idx];
  }

  if (part2) {
    bool *swap_grid = malloc(size);
    memset(swap_grid, 0, size);

    for (int day = 1; day <= 100; day++) {
      for (int q = -farthest_point + 2; q < (int)farthest_point - 1; q++) {
        for (int r = -farthest_point + 2; r < (int)farthest_point - 1; r++) {
          struct point p = {.q = q, .r = r};
          size_t idx = idx_grid(p, farthest_point);

          bool black = grid[idx];
          unsigned black_neighbors = count_black_neighbors(p, grid, farthest_point);

          if (black) {
            if (black_neighbors == 0 || black_neighbors > 2) {
              swap_grid[idx] = false;
            } else {
              swap_grid[idx] = true;
            }
          } else {
            if (black_neighbors == 2) {
              swap_grid[idx] = true;
            } else {
              swap_grid[idx] = false;
            }
          }
        }
      }

      bool *tmp = grid;
      grid = swap_grid;
      swap_grid = tmp;
    }

    free(swap_grid);
  }

  unsigned count = 0;
  for (int q = -farthest_point + 1; q < (int)farthest_point; q++) {
    for (int r = -farthest_point + 1; r < (int)farthest_point; r++) {
      struct point p = {.q = q, .r = r};
      size_t idx = idx_grid(p, farthest_point);
      if (grid[idx]) {
        count++;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
  for (size_t i = 0;; i++) {
    if (directions[i] == NULL) {
      break;
    }
    free(directions[i]);
  }
  free(directions);
  free(grid);
}

static void solution1(const char *const input, char *const output) { solution(input, output, false); }

static void solution2(const char *const input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
