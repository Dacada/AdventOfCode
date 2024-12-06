#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// #define VERBOSE

#define IDX(x, y) ((y)*7 + (x))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

// extra line to make falling easier
static const int rocks[5][7 * 8] = {
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0,
        0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    },
};

static const int rock_extra_height[5] = {3, 1, 1, 0, 2};

static void print_stage(const int *stage, int len) {
#ifdef VERBOSE
  for (int j = len - 1; j >= 0; j--) {
    for (int i = 0; i < 7; i++) {
      int x = stage[IDX(i, j)];
      char c;
      if (x == 0) {
        c = '.';
      } else if (x == 1) {
        c = '@';
      } else if (x == 2) {
        c = '#';
      } else {
        FAIL("invalid value '%d' at %d,%d", x, i, j);
      }
      fputc(c, stderr);
    }
    fputc('\n', stderr);
  }
  fputc('\n', stderr);
#else
  (void)stage;
  (void)len;
#endif
}

static int do_push(int *stage, int bottom, int top, char dir) {
  // DBG("%c", dir);
  for (int j = bottom; j < top; j++) {
    if (dir == '<') {
      if (stage[IDX(0, j)] == 1) {
        return 0;
      }
      for (int i = 0; i < 6; i++) {
        int x = stage[IDX(i + 1, j)];
        int y = stage[IDX(i, j)];
        if (x == 1 && y == 2) {
          return 0;
        }
      }
    } else {
      if (stage[IDX(6, j)] == 1) {
        return 0;
      }
      for (int i = 6; i >= 1; i--) {
        int x = stage[IDX(i - 1, j)];
        int y = stage[IDX(i, j)];
        if (x == 1 && y == 2) {
          return 0;
        }
      }
    }
  }

  for (int j = bottom; j < top; j++) {
    if (dir == '<') {
      for (int i = 0; i < 6; i++) {
        int x = stage[IDX(i + 1, j)];
        int y = stage[IDX(i, j)];
        if (y == 2) {
          continue;
        }
        if (x == 2) {
          x = 0;
        }
        stage[IDX(i, j)] = x;
      }
      if (stage[IDX(6, j)] != 2) {
        stage[IDX(6, j)] = 0;
      }
    } else {
      for (int i = 6; i >= 1; i--) {
        int x = stage[IDX(i - 1, j)];
        int y = stage[IDX(i, j)];
        if (y == 2) {
          continue;
        }
        if (x == 2) {
          x = 0;
        }
        stage[IDX(i, j)] = x;
      }
      if (stage[IDX(0, j)] != 2) {
        stage[IDX(0, j)] = 0;
      }
    }
  }

  return dir == '<' ? -1 : +1;
}

static bool do_fall(int *stage, int bottom, int top) {
  if (bottom == 0) {
    return false;
  }

  for (int i = 0; i < 7; i++) {
    for (int j = bottom; j < top; j++) {
      if (stage[IDX(i, j)] == 1 && stage[IDX(i, j - 1)] == 2) {
        return false;
      }
    }
  }

  for (int i = 0; i < 7; i++) {
    for (int j = bottom; j <= top; j++) {
      int x = stage[IDX(i, j)];
      int y = stage[IDX(i, j - 1)];
      if (y == 2) {
        continue;
      }
      if (x == 2) {
        x = 0;
      }
      stage[IDX(i, j - 1)] = x;
    }
  }
  return true;
}

static void skip_newlines(const char *input, int *i) {
  while (input[*i] == '\n') {
    *i += 1;
  }
}

static void update(int *stage, int *length, const char *input, int *input_idx, int rockidx) {
  int len = *length;

  int newlen = len + 7;
  memcpy(stage + len * 7, rocks[rockidx], sizeof(*rocks));
  newlen -= rock_extra_height[rockidx];

  int piece_bottom = len + 3;
  int piece_top = newlen;
  for (;;) {
    // DBG("%d %d %d %d", piece_bottom, piece_top, len, newlen);
    print_stage(stage, newlen);
    int i = *input_idx;
    skip_newlines(input, &i);
    char c = input[i];
    if (c == '\0') {
      c = input[0];
      i = 0;
    }
    i += 1;
    *input_idx = i;

    do_push(stage, piece_bottom, piece_top, c);
    print_stage(stage, newlen);
    if (do_fall(stage, piece_bottom, piece_top)) {
      piece_bottom--;
      piece_top--;
      newlen = MAX(piece_top, len);
    } else {
      break;
    }
  }

  for (int j = piece_bottom; j < newlen; j++) {
    for (int i = 0; i < 7; i++) {
      if (stage[IDX(i, j)] == 1) {
        stage[IDX(i, j)] = 2;
      }
    }
  }

  *length = newlen;
}

static void solution1(const char *input, char *const output) {
  int len = 0;
  static int stage[1 << 15];

  int idx = 0;
  for (int rock = 0; rock < 2022; rock++) {
    update(stage, &len, input, &idx, rock % 5);
    print_stage(stage, len);
    DBG("%d", len);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", len);
}

static void reverse(int *array, int len) {
  for (int i = 0; i < len / 2; i++) {
    int tmp = array[i];
    array[i] = array[len - i - 1];
    array[len - i - 1] = tmp;
  }
}

#define LEN_SIMULATION 5000

static void solution2(const char *const input, char *const output) {
  int len = 0;
  static int stage[1 << 16];
  static int height_diffs[LEN_SIMULATION];

  int idx = 0;
  int prev_len = 0;
  for (int rock = 0; rock < LEN_SIMULATION; rock++) {
    update(stage, &len, input, &idx, rock % 5);
    height_diffs[rock] = len - prev_len;
    prev_len = len;
    print_stage(stage, len);
  }
  reverse(height_diffs, LEN_SIMULATION);

  bool found = false;
  long pattern_len;
  for (pattern_len = 5; pattern_len * 2 < LEN_SIMULATION; pattern_len++) {
    if (memcmp(height_diffs, height_diffs + pattern_len, pattern_len * sizeof(*height_diffs)) == 0) {
      DBG("len=%ld", pattern_len);
      found = true;
      break;
    }
  }
  ASSERT(found, "no pattern");

  long height_increase = 0;
  for (int i = 0; i < pattern_len; i++) {
    height_increase += height_diffs[i];
  }
  DBG("inc=%ld", height_increase);

  long pattern_start;
  for (pattern_start = 0; pattern_start + pattern_len * 2 < LEN_SIMULATION; pattern_start += pattern_len) {
    if (memcmp(height_diffs + pattern_start, height_diffs + pattern_start + pattern_len,
               pattern_len * sizeof(*height_diffs)) != 0) {
      break;
    }
  }
  pattern_start += pattern_len;
  pattern_start = LEN_SIMULATION - pattern_start;
  DBG("start=%ld", pattern_start);

  long goal = 1000000000000;
  long remaining = goal - pattern_start;

  // height before pattern start
  reverse(height_diffs, LEN_SIMULATION);
  long height = 0;
  for (int i = 0; i < pattern_start; i++) {
    height += height_diffs[i];
  }

  long patterns_remaining = remaining / pattern_len;
  long extra_steps = remaining % pattern_len;
  DBG("%ld = %ld + %ld*%ld + %ld", goal, pattern_start, pattern_len, patterns_remaining, extra_steps);

  height += height_increase * patterns_remaining;

  for (int i = 0; i < extra_steps; i++) {
    height += height_diffs[i + pattern_start];
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", height);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
