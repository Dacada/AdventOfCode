#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

#define CHAR_IDX(a, b) ((int)(a)*256 + (int)(b))
#define CHAR_MAP_SIZE (256 * 256)

static const int numpad_w = 3;
static const int numpad_h = 4;
static const char *numpad = "789"
                            "456"
                            "123"
                            " 0A";

static const int dirpad_w = 3;
static const int dirpad_h = 2;
static const char *dirpad = " ^A"
                            "<v>";

struct string {
  const char *s;
  size_t len;
};

struct path {
  enum aoc_direction *dirs;
  size_t ndirs;
  int memoization_idx;
};

struct path_list {
  struct path *paths;
  size_t npaths;
};

struct memory {
  long *by_depth;
};

static struct string *parse_input(const char *input, int *len) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(struct string), 4);

  while (*input != '\0') {
    struct string *str = aoc_dynarr_grow(&arr, 1);
    str->s = input;
    str->len = 0;
    while (*input != '\n' && *input != '\0') {
      str->len++;
      input++;
    }
    while (*input == '\n') {
      input++;
    }
  }

  *len = arr.len;
  return arr.data;
}

static struct path_list *find_optimal_paths(const char *keypad, int width, int height, struct aoc_dynarr *memory) {
  struct path_list *map = aoc_malloc(CHAR_MAP_SIZE * sizeof(*map));

  if (memory != NULL) {
    aoc_dynarr_init(memory, sizeof(struct memory), 64);
  }

  for (int ay = 0; ay < height; ay++) {
    for (int ax = 0; ax < width; ax++) {
      char a = keypad[AOC_2D_IDX(ax, ay, width)];
      if (a == ' ') {
        continue;
      }
      for (int by = 0; by < height; by++) {
        for (int bx = 0; bx < width; bx++) {
          char b = keypad[AOC_2D_IDX(bx, by, width)];
          if (b == ' ') {
            continue;
          }

          struct path_list *path_list = &map[CHAR_IDX(a, b)];
          path_list->paths = aoc_malloc(sizeof(*path_list->paths) * 2);
          struct path *p1 = &path_list->paths[0];
          struct path *p2 = &path_list->paths[1];

          // We're already there, best path is to do nothing
          if (a == b) {
            p1->ndirs = 0;
            p1->dirs = NULL;
            if (memory != NULL) {
              p1->memoization_idx = memory->len;
              aoc_dynarr_grow(memory, 1);
            }
            path_list->npaths = 1;
            continue;
          }

          int dx = bx - ax;
          int dy = by - ay;

          enum aoc_direction xdir, ydir;
          if (dx < 0) {
            xdir = AOC_DIRECTION_WEST;
          } else {
            xdir = AOC_DIRECTION_EAST;
          }
          if (dy < 0) {
            ydir = AOC_DIRECTION_NORTH;
          } else {
            ydir = AOC_DIRECTION_SOUTH;
          }

          // Already aligned there, only path is a straight line
          if (dx == 0) {
            p1->ndirs = AOC_ABS(dy);
            p1->dirs = aoc_malloc(sizeof(*p1->dirs) * p1->ndirs);
            for (unsigned i = 0; i < p1->ndirs; i++) {
              p1->dirs[i] = ydir;
            }
            if (memory != NULL) {
              p1->memoization_idx = memory->len;
              aoc_dynarr_grow(memory, 1);
            }
            path_list->npaths = 1;
            continue;
          }
          if (dy == 0) {
            p1->ndirs = AOC_ABS(dx);
            p1->dirs = aoc_malloc(sizeof(*p1->dirs) * p1->ndirs);
            for (unsigned i = 0; i < p1->ndirs; i++) {
              p1->dirs[i] = xdir;
            }
            if (memory != NULL) {
              p1->memoization_idx = memory->len;
              aoc_dynarr_grow(memory, 1);
            }
            path_list->npaths = 1;
            continue;
          }

          // Two possible paths, along x then along y, or along y then along x
          p1->ndirs = AOC_ABS(dx) + AOC_ABS(dy);
          p1->dirs = aoc_malloc(sizeof(*p1->dirs) * p1->ndirs);
          p2->ndirs = AOC_ABS(dy) + AOC_ABS(dx);
          p2->dirs = aoc_malloc(sizeof(*p2->dirs) * p2->ndirs);
          for (int i = 0; i < AOC_ABS(dx); i++) {
            p1->dirs[i] = xdir;
            p2->dirs[AOC_ABS(dy) + i] = xdir;
          }
          for (int i = 0; i < AOC_ABS(dy); i++) {
            p1->dirs[AOC_ABS(dx) + i] = ydir;
            p2->dirs[i] = ydir;
          }

          // Moving along one of the axis hits the gap, which is forbidden, we
          // actually only have one path. These two checks are enough only
          // because gaps are in the corners of the keypad
          if (keypad[AOC_2D_IDX(bx, ay, width)] == ' ') {
            free(p1->dirs);
            *p1 = *p2;
            path_list->npaths = 1;
            if (memory != NULL) {
              p1->memoization_idx = memory->len;
              aoc_dynarr_grow(memory, 1);
            }
            continue;
          }
          if (keypad[AOC_2D_IDX(ax, by, width)] == ' ') {
            free(p2->dirs);
            path_list->npaths = 1;
            if (memory != NULL) {
              p1->memoization_idx = memory->len;
              aoc_dynarr_grow(memory, 1);
            }
            continue;
          }

          // Base case: both paths are, in theory, optimal
          path_list->npaths = 2;
          if (memory != NULL) {
            p1->memoization_idx = memory->len;
            p2->memoization_idx = memory->len + 1;
            aoc_dynarr_grow(memory, 2);
          }
        }
      }
    }
  }

  return map;
}

static void free_optimal_paths(struct path_list *path_lists, int width, int height, const char *keypad) {
  for (int i = 0; i < width * height; i++) {
    char a = keypad[i];
    if (a == ' ') {
      continue;
    }
    for (int j = 0; j < width * height; j++) {
      char b = keypad[j];
      if (b == ' ') {
        continue;
      }
      struct path_list list = path_lists[CHAR_IDX(a, b)];
      for (unsigned k = 0; k < list.npaths; k++) {
        struct path path = list.paths[k];
        free(path.dirs);
      }
      free(list.paths);
    }
  }
  free(path_lists);
}

__attribute__((pure)) static long get_int_from_code(const struct string code) {
  long n = 0;
  for (unsigned i = 0; i < code.len; i++) {
    char c = code.s[i];
    if (!isdigit(c)) {
      continue;
    }
    n *= 10;
    n += c - '0';
  }
  return n;
}

static long get_num_presses(const struct string code, int depth, const struct path_list *path_list_grid,
                            const struct path_list *other_path_list_grid, bool use_other_grid, struct memory *memory) {
  const struct path_list *grid;
  if (use_other_grid) {
    grid = other_path_list_grid;
  } else {
    grid = path_list_grid;
  }

  if (depth == 1) {
    return code.len;
  }

  long sum = 0;
  for (unsigned i = 0; i < code.len; i++) {
    char a;
    if (i == 0) {
      a = 'A';
    } else {
      a = code.s[i - 1];
    }
    char b = code.s[i];

    long min = LONG_MAX;
    struct path_list path_list = grid[CHAR_IDX(a, b)];
    for (unsigned n = 0; n < path_list.npaths; n++) {
      struct path path = path_list.paths[n];

      bool should_compute = false;
      long *memory_cell = NULL;
      if (use_other_grid) {
        should_compute = true;
      } else {
        struct memory mem = memory[path.memoization_idx];
        memory_cell = &mem.by_depth[depth - 1];
        if (*memory_cell == -1) {
          should_compute = true;
        }
      }

      long res;
      if (should_compute) {
        char *s = aoc_malloc(sizeof(*s) * (path.ndirs + 1));
        for (unsigned j = 0; j < path.ndirs; j++) {
          s[j] = "^>v<"[path.dirs[j]];
        }
        s[path.ndirs] = 'A';
        struct string newcode = {.s = s, .len = path.ndirs + 1};
        DBG("%.*s", (int)newcode.len, newcode.s);
        res = get_num_presses(newcode, depth - 1, path_list_grid, other_path_list_grid, false, memory);
        free(s);
        if (memory_cell != NULL) {
          *memory_cell = res;
        }
      } else {
        res = *memory_cell;
      }

      if (res < min) {
        min = res;
      }
    }

    sum += min;
  }

  return sum;
}

static void solution(const char *input, char *const output, int depth) {
  int ncodes;
  struct string *codes = parse_input(input, &ncodes);

  struct path_list *numpad_path_lists = find_optimal_paths(numpad, numpad_w, numpad_h, NULL);
  struct aoc_dynarr memory;
  struct path_list *dirpad_path_lists = find_optimal_paths(dirpad, dirpad_w, dirpad_h, &memory);

  for (int i = 0; i < memory.len; i++) {
    struct memory *mem = &((struct memory *)memory.data)[i];
    mem->by_depth = aoc_malloc(sizeof(*mem->by_depth) * (depth + 1));
    for (int j = 0; j < depth + 1; j++) {
      mem->by_depth[j] = -1;
    }
  }

  long res = 0;
  for (int i = 0; i < ncodes; i++) {
    long p = get_num_presses(codes[i], depth, dirpad_path_lists, numpad_path_lists, true, memory.data);
    res += p * get_int_from_code(codes[i]);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
  free(codes);
  free_optimal_paths(numpad_path_lists, numpad_w, numpad_h, numpad);
  free_optimal_paths(dirpad_path_lists, dirpad_w, dirpad_h, dirpad);
  for (int i = 0; i < memory.len; i++) {
    free(AOC_DYNARR_IDX(memory, i, struct memory).by_depth);
  }
  aoc_dynarr_free(&memory);
}

static void solution1(const char *input, char *const output) { solution(input, output, 4); }

static void solution2(const char *input, char *const output) { solution(input, output, 27); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
