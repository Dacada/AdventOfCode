#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define IDX(map, x, y, width) ((map)[(x) + (y) * (width)])

static char *parse_input(const char *input, int *width, int *height) {
  int len = 0;
  int cap = 16;
  char *list = malloc(sizeof(*list) * cap);

  int w = 0;
  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    if (*input == '\n') {
      if (w == 0) {
        w = len;
      }
      input += 1;
      continue;
    }
    list[len++] = *input;
    input += 1;
  }

  *width = w;
  *height = len / w;
  return list;
}

static int north_load(const char *map, int width, int height) {
  int res = 0;
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      if (IDX(map, i, j, width) == 'O') {
        res += height - j;
      }
    }
  }
  return res;
}

static void tilt_north(char *map, int width, int height) {
  for (int i = 0; i < width; i++) {
    int y = 0;
    for (int j = 0; j < height; j++) {
      char c = IDX(map, i, j, width);
      if (c == '#') {
        y = j + 1;
      } else if (c == 'O') {
        IDX(map, i, j, width) = '.';
        IDX(map, i, y, width) = 'O';
        y += 1;
      }
    }
  }
}

static void tilt_west(char *map, int width, int height) {
  for (int j = 0; j < height; j++) {
    int x = 0;
    for (int i = 0; i < width; i++) {
      char c = IDX(map, i, j, width);
      if (c == '#') {
        x = i + 1;
      } else if (c == 'O') {
        IDX(map, i, j, width) = '.';
        IDX(map, x, j, width) = 'O';
        x += 1;
      }
    }
  }
}

static void tilt_south(char *map, int width, int height) {
  for (int i = 0; i < width; i++) {
    int y = height - 1;
    for (int j = height - 1; j >= 0; j--) {
      char c = IDX(map, i, j, width);
      if (c == '#') {
        y = j - 1;
      } else if (c == 'O') {
        IDX(map, i, j, width) = '.';
        IDX(map, i, y, width) = 'O';
        y -= 1;
      }
    }
  }
}

static void tilt_east(char *map, int width, int height) {
  for (int j = 0; j < height; j++) {
    int x = width - 1;
    for (int i = width - 1; i >= 0; i--) {
      char c = IDX(map, i, j, width);
      if (c == '#') {
        x = i - 1;
      } else if (c == 'O') {
        IDX(map, i, j, width) = '.';
        IDX(map, x, j, width) = 'O';
        x -= 1;
      }
    }
  }
}

static void cycle(char *map, int width, int height) {
  tilt_north(map, width, height);
  tilt_west(map, width, height);
  tilt_south(map, width, height);
  tilt_east(map, width, height);
}

static void solution1(const char *const input, char *const output) {
  int width, height;
  char *map = parse_input(input, &width, &height);

#ifdef DEBUG
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      fputc(IDX(map, i, j, width), stderr);
    }
    fputc('\n', stderr);
  }
#endif

  tilt_north(map, width, height);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", north_load(map, width, height));
  free(map);
}

static bool array_contains(char **array, const char *key, int arrlen, int keylen) {
  for (int i = 0; i < arrlen; i++) {
    if (strncmp(array[i], key, keylen) == 0) {
      return true;
    }
  }
  return false;
}

static int array_index(char **array, char *key, int arrlen, int keylen) {
  for (int i = 0; i < arrlen; i++) {
    if (strncmp(array[i], key, keylen) == 0) {
      return i;
    }
  }
  FAIL("not found");
}

static void solution2(const char *const input, char *const output) {
  int width, height;
  char *map = parse_input(input, &width, &height);

  long lim = 1000000000L;
  static char *strings[1 << 10];
  int i;
  for (i = 0; i < lim; i++) {
    cycle(map, width, height);
    if (array_contains(strings, map, i, width * height)) {
      strings[i] = strndup(map, width * height);
      break;
    }
    strings[i] = strndup(map, width * height);
  }
  int len = i + 1;

  int starts = array_index(strings, map, len, width * height);
  int period = array_index(strings + starts + 1, map, len - starts - 1, width * height) + 1;
  char *goal = strings[starts + (lim - starts) % period - 1];

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", north_load(goal, width, height));
  free(map);
  for (i = 0; i < len; i++) {
    free(strings[i]);
  }
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
