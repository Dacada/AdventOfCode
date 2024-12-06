#include <aoclib.h>
#include <stdio.h>
#include <string.h>

struct set {
  size_t length;
  size_t size;
  bool *elements;
};

static void set_init(struct set *const set) {
  set->length = 0;
  set->size = 16;
  set->elements = malloc(set->size * sizeof(*set->elements));
}
static void set_initFrom(struct set *const set, const struct set *const other) {
  set->length = other->length;
  set->size = other->size;
  set->elements = malloc(set->size * sizeof(*set->elements));
  memcpy(set->elements, other->elements, set->size * sizeof(*set->elements));
}
static void set_free(struct set *const set) {
  free(set->elements);
  set->elements = NULL;
}
static void set_add(struct set *const set, unsigned element) {
  size_t old_length = set->length;
  if (element >= set->length) {
    set->length = element + 1;
  }

  bool resize = false;
  while (set->size <= set->length) {
    resize = true;
    set->size *= 2;
  }
  if (resize) {
    set->elements = realloc(set->elements, set->size * sizeof(set->elements));
  }

  for (size_t i = old_length; i < set->length; i++) {
    set->elements[i] = false;
  }
  set->elements[element] = true;
}
static bool set_contains(const struct set *const set, unsigned element) {
  if (set->length <= element) {
    return false;
  }
  return set->elements[element];
}

struct maze {
  unsigned start;
  unsigned end;

  unsigned ncaves;
  struct set *connections;
  struct set bigcave;
};

static unsigned encode_name(const char **const input, bool *const big, bool *const start, bool *const end) {
  unsigned n;
  if (strncmp(*input, "start", 5) == 0) {
    *start = true;
    *end = false;
    *big = false;
    n = ('z' - 'a') * 26 + ('z' - 'a') + 1;
    *input += 5;
  } else if (strncmp(*input, "end", 3) == 0) {
    *start = false;
    *end = true;
    *big = false;
    n = ('z' - 'a') * 26 + ('z' - 'a') + 2;
    *input += 3;
  } else {
    *start = false;
    *end = false;

    if (**input >= 'A' && **input <= 'Z') {
      *big = true;
    } else {
      *big = false;
    }

    if (*big) {
      n = (**input - 'A') * 26;
    } else {
      n = (**input - 'a') * 26;
    }
    *input += 1;
    ASSERT((*big && (**input >= 'A' && **input <= 'Z')) || (!*big && (**input >= 'a' && **input <= 'z')),
           "parse error");
    if (*big) {
      n += **input - 'A';
    } else {
      n += **input - 'a';
    }
    *input += 1;
  }
  return n;
}

static unsigned parse_cave_name(const char **const input, bool *const big, bool *const start, bool *const end) {
  static unsigned *names;
  static size_t size;
  static unsigned len;
  static bool first = true;
  if (first) {
    size = 16;
    len = 0;
    names = malloc(sizeof(*names) * size);
    first = false;
  }

  unsigned name = encode_name(input, big, start, end);

  for (unsigned i = 0; i < len; i++) {
    if (names[i] == name) {
      return i;
    }
  }

  unsigned ret = len;
  len += 1;
  if (len >= size) {
    size *= 2;
    names = realloc(names, sizeof(*names) * size);
  }
  names[ret] = name;
  return ret;
}

static unsigned handle_cave(const char **const input, struct maze *const maze, size_t *const cavessize) {
  bool big, start, end;
  unsigned cave = parse_cave_name(input, &big, &start, &end);

  bool newcave = cave >= maze->ncaves;
  if (newcave) {
    maze->ncaves++;
  }
  if (maze->ncaves >= *cavessize) {
    *cavessize *= 2;
    maze->connections = realloc(maze->connections, *cavessize * sizeof(*maze->connections));
  }
  if (newcave) {
    set_init(&maze->connections[cave]);
    if (big) {
      set_add(&maze->bigcave, cave);
    }
  }

  if (start) {
    maze->start = cave;
  } else if (end) {
    maze->end = cave;
  }

  return cave;
}

static void parse_input(const char *input, struct maze *const maze) {
  size_t cavessize = 16;
  maze->connections = malloc(cavessize * sizeof(*maze->connections));
  set_init(&maze->bigcave);

  maze->ncaves = 0;

  while (*input != '\0') {
    unsigned cave1 = handle_cave(&input, maze, &cavessize);
    ASSERT(*input == '-', "parse error");
    input++;
    unsigned cave2 = handle_cave(&input, maze, &cavessize);
    ASSERT(*input == '\n' || *input == '\0', "parse error");
    if (*input == '\n') {
      input++;
    }
    set_add(&maze->connections[cave1], cave2);
    set_add(&maze->connections[cave2], cave1);
  }
}

static unsigned count_paths(const struct maze *const maze, unsigned from, const struct set *const visited,
                            bool small_cave_visited_twice, char *dbg_path, int dbg_path_i) {
  unsigned count = 0;

  struct set newvisited;
  set_initFrom(&newvisited, visited);
  set_add(&newvisited, from);

  for (unsigned cave = 0; cave < maze->ncaves; cave++) {
    if (set_contains(&maze->connections[from], cave)) {
      bool visited_twice = small_cave_visited_twice;
      if (!set_contains(&maze->bigcave, cave) && set_contains(&newvisited, cave)) {
        if (!small_cave_visited_twice && cave != maze->start) {
          visited_twice = true;
        } else {
          continue;
        }
      }

      dbg_path[dbg_path_i] = cave + '0';
      if (cave == maze->end) {
        dbg_path[dbg_path_i + 1] = '\0';
        // DBG("%s", dbg_path);
        count++;
      } else {
        count += count_paths(maze, cave, &newvisited, visited_twice, dbg_path, dbg_path_i + 1);
      }
    }
  }

  set_free(&newvisited);

  return count;
}

static void solution(const char *const input, char *const output, bool strictly_once) {
  struct maze maze;
  parse_input(input, &maze);

  struct set visited;
  set_init(&visited);
  char dbg_path[256];
  dbg_path[0] = maze.start + '0';
  unsigned paths = count_paths(&maze, maze.start, &visited, strictly_once, dbg_path, 1);
  set_free(&visited);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", paths);
  for (unsigned i = 0; i < maze.ncaves; i++) {
    set_free(&maze.connections[i]);
  }
  free(maze.connections);
  set_free(&maze.bigcave);
}

static void solution1(const char *const input, char *const output) { solution(input, output, true); }

static void solution2(const char *const input, char *const output) { solution(input, output, false); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
