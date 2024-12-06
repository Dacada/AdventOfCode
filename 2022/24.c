#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define IDX(i, j) ((j)*width + (i))

enum cell_type {
  CELL_WALL,
  CELL_GROUND,
};

enum direction {
  NORTH,
  SOUTH,
  WEST,
  EAST,
};

struct cell {
  enum cell_type type;
  enum direction blizards[1 << 6];
  int nblizards;
};

static struct cell *parse_input(const char *input, int *width, int *height) {
  int cap = 32;
  int len = 0;
  struct cell *map = malloc(sizeof(*map) * cap);

  int w = 0;
  *height = 0;
  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      map = realloc(map, sizeof(*map) * cap);
    }
    char c = *input;
    if (c == '\n') {
      *width = w;
      w = 0;
      *height += 1;
      input++;
      while (*input == '\n') {
        input++;
      }
    } else {
      if (c == '#') {
        map[len].type = CELL_WALL;
      } else {
        map[len].type = CELL_GROUND;
        if (c == '.') {
          map[len].nblizards = 0;
        } else {
          map[len].nblizards = 1;
          if (c == '^') {
            map[len].blizards[0] = NORTH;
          } else if (c == 'v') {
            map[len].blizards[0] = SOUTH;
          } else if (c == '<') {
            map[len].blizards[0] = WEST;
          } else if (c == '>') {
            map[len].blizards[0] = EAST;
          } else {
            FAIL("parse error");
          }
        }
      }
      len++;
      input++;
      w++;
    }
  }

  return map;
}

struct state {
  int minutes;
  int posx;
  int posy;
};

static void advance_pos(int where, int *x, int *y) {
  if (where == 4) {
    return;
  } else if (where > 4 || where < 0) {
    FAIL("invalid advance direction");
  }
  enum direction dir = where;
  switch (dir) {
  case NORTH:
    *y -= 1;
    break;
  case SOUTH:
    *y += 1;
    break;
  case WEST:
    *x -= 1;
    break;
  case EAST:
    *x += 1;
    break;
  }
}

static bool is_pos_valid(const struct cell *map, int width, int height, int x, int y) {
  if (x < 0 || y < 0 || x >= width || y >= height) {
    return false;
  }
  if (map[IDX(x, y)].type == CELL_WALL) {
    return false;
  }
  if (map[IDX(x, y)].nblizards > 0) {
    return false;
  }
  return true;
}

static struct cell *advance_map(const struct cell *map, int width, int height) {
  struct cell *newmap = malloc(sizeof(*newmap) * width * height);
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      const struct cell *cell = &map[IDX(i, j)];
      struct cell *newcell = &newmap[IDX(i, j)];
      newcell->type = cell->type;
      newcell->nblizards = 0;
    }
  }
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      const struct cell *cell = &map[IDX(i, j)];
      if (cell->type == CELL_WALL) {
        continue;
      }
      for (int k = 0; k < cell->nblizards; k++) {
        int ii = i;
        int jj = j;
        enum direction dir = cell->blizards[k];
        advance_pos(dir, &ii, &jj);
        if (newmap[IDX(ii, jj)].type == CELL_WALL) {
          switch (dir) {
          case NORTH:
            jj = height - 2;
            break;
          case SOUTH:
            jj = 1;
            break;
          case WEST:
            ii = width - 2;
            break;
          case EAST:
            ii = 1;
            break;
          }
        }
        struct cell *newcell = &newmap[IDX(ii, jj)];
        ASSERT(newcell->type == CELL_GROUND, "invalid blizzard position %d,%d (%d,%d)", ii, jj, width, height);
        newcell->blizards[newcell->nblizards++] = dir;
      }
    }
  }

  return newmap;
}

static void prnt_map(const struct cell *map, int width, int height, bool *pos, bool assert) {
#ifdef DEBUG
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      const struct cell *cell = &map[IDX(i, j)];
      char c;
      switch (cell->type) {
      case CELL_WALL:
        c = '#';
        break;
      case CELL_GROUND:
        c = '.';
        if (cell->nblizards == 1) {
          switch (cell->blizards[0]) {
          case NORTH:
            c = '^';
            break;
          case SOUTH:
            c = 'v';
            break;
          case WEST:
            c = '<';
            break;
          case EAST:
            c = '>';
            break;
          default:
            FAIL("parse error");
          }
        } else if (cell->nblizards > 1) {
          if (assert) {
            FAIL("parse error");
          } else {
            c = cell->nblizards + '0';
          }
        }
        break;
      default:
        FAIL("parse error");
      }
      if (pos != NULL && pos[IDX(i, j)]) {
        c = 'E';
      }
      fputc(c, stderr);
    }
    fputc('\n', stderr);
  }
  fputc('\n', stderr);
#else
  (void)map;
  (void)width;
  (void)height;
  (void)pos;
  (void)assert;
#endif
}

static int manhattan(int ax, int ay, int bx, int by) { return abs(ax - bx) + abs(ay - by); }

static void find_start_end(struct cell *map, int width, int height, int *startx, int *starty, int *endx, int *endy) {
  *starty = 0;
  *endy = height - 1;
  for (int i = 0; i < width; i++) {
    if (map[IDX(i, *starty)].type == CELL_GROUND) {
      *startx = i;
    }
    if (map[IDX(i, *endy)].type == CELL_GROUND) {
      *endx = i;
    }
  }
}

static int search(struct cell **maps, int width, int height, int maxmaps, int startx, int starty, int endx, int endy,
                  int firstminute) {
  bool *current = malloc(sizeof(*current) * width * height);
  memset(current, 0, sizeof(*current) * width * height);
  bool *next = malloc(sizeof(*next) * width * height);
  memset(next, 0, sizeof(*next) * width * height);

  current[IDX(startx, starty)] = true;
  for (int minute = firstminute; minute < maxmaps; minute++) {
    const struct cell *map = maps[minute];

    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        if (!current[IDX(i, j)]) {
          continue;
        }
        for (int k = 0; k < 5; k++) {
          int ii = i;
          int jj = j;
          advance_pos(k, &ii, &jj);
          (void)manhattan;
          /* if (manhattan(ii, jj, endx, endy) > maxmaps-minute) { */
          /*         continue; */
          /* } */
          if (is_pos_valid(map, width, height, ii, jj)) {
            next[IDX(ii, jj)] = true;
            if (ii == endx && jj == endy) {
              free(current);
              free(next);
              return minute;
            }
          }
        }
      }
    }

    DBG("Minute %d", minute + 1);
    prnt_map(map, width, height, next, false);

    bool *tmp = current;
    current = next;
    next = tmp;
    memset(next, 0, sizeof(*next) * width * height);
  }

  FAIL("not found");
}

static void solution1(const char *const input, char *const output) {
  int width = -1;
  int height = -1;
  struct cell *map = parse_input(input, &width, &height);

  int startx = -1;
  int starty = -1;
  int endx = -1;
  int endy = -1;
  find_start_end(map, width, height, &startx, &starty, &endx, &endy);
  ASSERT(startx >= 0 && endx >= 0 && starty >= 0 && endx >= 0, "parse error");

  prnt_map(map, width, height, NULL, true);

  const int maxminutes = 400;
  struct cell **maps = malloc(sizeof(**maps) * width * height * maxminutes);
  maps[0] = map;
  for (int i = 1; i < maxminutes; i++) {
    maps[i] = advance_map(maps[i - 1], width, height);
  }

  int res = search(maps, width, height, maxminutes, startx, starty, endx, endy, 1);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  for (int i = 0; i < maxminutes; i++) {
    free(maps[i]);
  }
  free(maps);
}

static void solution2(const char *const input, char *const output) {
  int width = -1;
  int height = -1;
  struct cell *map = parse_input(input, &width, &height);

  int startx = -1;
  int starty = -1;
  int endx = -1;
  int endy = -1;
  find_start_end(map, width, height, &startx, &starty, &endx, &endy);
  ASSERT(startx >= 0 && endx >= 0 && starty >= 0 && endx >= 0, "parse error");

  const int maxminutes = 1000;
  struct cell **maps = malloc(sizeof(**maps) * width * height * maxminutes);
  maps[0] = map;
  for (int i = 1; i < maxminutes; i++) {
    maps[i] = advance_map(maps[i - 1], width, height);
  }

  int res = search(maps, width, height, maxminutes, startx, starty, endx, endy, 1);
  res = search(maps, width, height, maxminutes, endx, endy, startx, startx, res + 1);
  res = search(maps, width, height, maxminutes, startx, starty, endx, endy, res + 1);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  for (int i = 0; i < maxminutes; i++) {
    free(maps[i]);
  }
  free(maps);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
