#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static int step(const char *manifold, int y, int width, long *timelines) {
  int splits = 0;

  for (int i = 0; i < width; i++) {
    char cell = manifold[AOC_2D_IDX(i, y, width)];

    long *curr = &timelines[AOC_2D_IDX(i, y, width)];
    long *prev = &timelines[AOC_2D_IDX(i, y - 1, width)];

    long *before_curr = NULL;
    long *after_curr = NULL;
    if (i > 0) {
      before_curr = &timelines[AOC_2D_IDX(i - 1, y, width)];
    }
    if (i < width - 1) {
      after_curr = &timelines[AOC_2D_IDX(i + 1, y, width)];
    }

    if (cell == '.') {
      *curr += *prev;
    } else if (cell == '^') {
      if (*prev > 0) {
        splits += 1;
      }
      if (before_curr != NULL) {
        *before_curr += *prev;
      }
      if (after_curr != NULL) {
        *after_curr += *prev;
      }
    }
  }

  /*
  #ifdef DEBUG
  for (int i = 0; i < width; i++) {
    fputc(timelines[AOC_2D_IDX(i, y, width)] + '0', stderr);
  }
  fputc('\n', stderr);
  #endif
  */

  return splits;
}

static void solution(const char *input, char *const output, bool many_worlds) {
  int height, width;
  char *manifold = aoc_parse_grid_chars(&input, &height, &width);

  long *timelines = NULL;
  {
    size_t size = sizeof(*timelines) * height * width;
    timelines = malloc(size);
    memset(timelines, 0, size);
  }

  for (int i = 0; i < width; i++) {
    char *ptr = &manifold[AOC_2D_IDX(i, 0, width)];
    if (*ptr == 'S') {
      timelines[AOC_2D_IDX(i, 0, width)] = 1;
      break;
    }
  }

  long total = 0;
  for (int j = 1; j < height; j++) {
    int splits = step(manifold, j, width, timelines);
    if (!many_worlds) {
      total += splits;
    }
  }

  if (many_worlds) {
    for (int i = 0; i < width; i++) {
      total += timelines[AOC_2D_IDX(i, height - 1, width)];
    }
  }

  free(manifold);
  free(timelines);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
