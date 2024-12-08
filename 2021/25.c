#include <aoclib.h>
#include <stdio.h>
#include <string.h>

enum cucumber {
  CUCUMBER_NONE,
  CUCUMBER_EAST,
  CUCUMBER_SOUTH,
};

__attribute__((pure)) static enum cucumber identify_cucumber(const char c) {
  if (c == '.') {
    return CUCUMBER_NONE;
  } else if (c == '>') {
    return CUCUMBER_EAST;
  } else if (c == 'v') {
    return CUCUMBER_SOUTH;
  } else {
    FAIL("parse error");
  }
}

static void parse_input_cb(const char **input, void *res, int x, int y, void *args) {
  (void)args;
  (void)x;
  (void)y;

  enum cucumber *result = res;
  *result = identify_cucumber(**input);
  *input += 1;
}

static enum cucumber *parse_input(const char *input, int *const width, int *const height) {
  return aoc_parse_grid(&input, parse_input_cb, sizeof(enum cucumber), height, width, NULL);
}

static void get_next_location(enum cucumber which, int i, int j, int *ii, int *jj, int width, int height) {
  if (which == CUCUMBER_EAST) {
    *ii = (i + 1) % width;
    *jj = j;
  } else if (which == CUCUMBER_SOUTH) {
    *ii = i;
    *jj = (j + 1) % height;
  } else {
    FAIL("bad cucumber");
  }
}

static bool step(enum cucumber *const grid, const int width, const int height, const enum cucumber which) {
  enum cucumber *const new_grid = malloc(sizeof(*new_grid) * width * height);
  memset(new_grid, 0, sizeof(*new_grid) * width * height);

  bool change = false;
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      enum cucumber c = grid[j * width + i];
      if (c == which) {
        int nexti, nextj;
        get_next_location(which, i, j, &nexti, &nextj, width, height);
        if (grid[nextj * width + nexti] == CUCUMBER_NONE) {
          new_grid[j * width + i] = CUCUMBER_NONE;
          new_grid[nextj * width + nexti] = which;
          change = true;
        } else {
          new_grid[j * width + i] = which;
        }
      } else if (c != CUCUMBER_NONE) {
        new_grid[j * width + i] = grid[j * width + i];
      }
    }
  }

  if (change) {
    memcpy(grid, new_grid, sizeof(*grid) * width * height);
  }
  free(new_grid);
  return change;
}

static void solution1(const char *const input, char *const output) {
  int width, height;
  enum cucumber *grid = parse_input(input, &width, &height);

  unsigned steps;
  for (steps = 0;; steps++) {
#ifdef DEBUG
    fprintf(stderr, "After %u steps:\n", steps);
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        enum cucumber cuc = grid[j * width + i];
        fprintf(stderr, "%c",
                cuc == CUCUMBER_NONE ? '.' : (cuc == CUCUMBER_EAST ? '>' : (cuc == CUCUMBER_SOUTH ? 'v' : '?')));
      }
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    bool east = step(grid, width, height, CUCUMBER_EAST);
    bool south = step(grid, width, height, CUCUMBER_SOUTH);
    if (!east && !south) {
      break;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", steps + 1);
  free(grid);
}

static void solution2(const char *const input, char *const output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "Checking Christmas status... Saved!");
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
