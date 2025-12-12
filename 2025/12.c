#include <aoclib.h>
#include <stdio.h>

static bool parse_present(const char **input, void *ptr) {
  if ((*input)[1] != ':') {
    return false;
  }

  aoc_parse_int(input);
  aoc_expect_text(input, ":\n", 2);

  int h, w;
  char *grid = aoc_parse_grid_chars(input, &h, &w);
  ASSERT(h == 3, "parse error");
  ASSERT(w == 3, "parse error");

  int total = 0;
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      if (grid[AOC_2D_IDX(i, j, w)] == '#') {
        total += 1;
      }
    }
  }
  free(grid);

  aoc_expect_text(input, "\n\n", 2);
  int *res = ptr;
  *res = total;
  return true;
}

static bool parse_int(const char **input, void *ptr) {
  if (**input == '\n' || **input == '\0') {
    return false;
  }
  int *n = ptr;
  *n = aoc_parse_int(input);
  if (**input == ' ') {
    *input += 1;
  }
  return true;
}

struct tree {
  int n, m;
  int npresents;
  int *presents;
};

static bool parse_tree(const char **input, void *ptr) {
  if (**input == '\0') {
    return false;
  }

  struct tree *res = ptr;
  res->n = aoc_parse_int(input);
  aoc_expect_char(input, 'x');
  res->m = aoc_parse_int(input);
  aoc_expect_text(input, ": ", 2);
  res->presents = aoc_parse_sequence(input, &res->npresents, sizeof(*res->presents), 4, parse_int);

  if (**input == '\n') {
    *input += 1;
  }
  return true;
}

static void solution1(const char *input, char *const output) {
  // As it turns out, the actual input is trivial even though the example is not...

  int npresents;
  int *presents = aoc_parse_sequence(&input, &npresents, sizeof(*presents), 8, parse_present);

  int ntrees;
  struct tree *trees = aoc_parse_sequence(&input, &ntrees, sizeof(*trees), 16, parse_tree);

  int res = 0;
  for (int i = 0; i < ntrees; i++) {
    struct tree tree = trees[i];
    int max_area = tree.n * tree.m;

    ASSERT(tree.npresents == npresents, "parse error");

    int used_area = 0;
    for (int j = 0; j < tree.npresents; j++) {
      used_area += tree.presents[j] * presents[j];
    }

    // trivial check, in theory this is only sufficient for rejection not acceptance, but...
    if (used_area <= max_area) {
      res += 1;
    }
  }

  free(presents);
  for (int i = 0; i < ntrees; i++) {
    free(trees[i].presents);
  }
  free(trees);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

static void solution2(const char *input, char *const output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "HUH??");
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
