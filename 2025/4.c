#include <aoclib.h>
#include <stdio.h>
#include <stdlib.h>

static int remove_rolls(char **rolls_ptr, int height, int width) {
  char *rolls = *rolls_ptr;
  char *new_rolls = malloc(sizeof(*new_rolls) * height * width);
  int removed = 0;

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      struct aoc_point p = {.x = i, .y = j};
      if (rolls[AOC_2D_IDX(p.x, p.y, width)] != '@') {
        new_rolls[AOC_2D_IDX(p.x, p.y, width)] = '.';
        continue;
      }

      int total = 0;
      for (int dj = -1; dj <= 1; dj++) {
        for (int di = -1; di <= 1; di++) {
          if (dj == 0 && di == 0) {
            continue;
          }
          struct aoc_point dp = p;
          dp.x += di;
          dp.y += dj;
          if (!aoc_point_in_limits(dp, width, height)) {
            continue;
          }
          if (rolls[AOC_2D_IDX(dp.x, dp.y, width)] == '@') {
            total += 1;
          }
        }
      }

      if (total < 4) {
        new_rolls[AOC_2D_IDX(p.x, p.y, width)] = '.';
        removed += 1;
      } else {
        new_rolls[AOC_2D_IDX(p.x, p.y, width)] = '@';
      }
    }
  }

  free(*rolls_ptr);
  *rolls_ptr = new_rolls;
  return removed;
}

static void solution1(const char *input, char *const output) {
  int height, width;
  char *rolls = aoc_parse_grid_chars(&input, &height, &width);

  int result = remove_rolls(&rolls, height, width);

  free(rolls);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *input, char *const output) {
  int height, width;
  char *rolls = aoc_parse_grid_chars(&input, &height, &width);

  int removed;
  int result = 0;
  do {
    removed = remove_rolls(&rolls, height, width);
    result += removed;
  } while (removed != 0);

  free(rolls);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
