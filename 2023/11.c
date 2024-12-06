#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define IDX(array, x, y, width) ((array)[(x) + (y) * (width)])
#define ABS(x) ((x) < 0 ? -(x) : (x))

struct point {
  int x;
  int y;
};

static char *parse_input(const char *input, int *width, int *height) {
  char *res = malloc(sizeof(*res) * strlen(input));
  *width = -1;
  int j = 0;
  for (int i = 0; input[i] != '\0'; i++) {
    char c = input[i];
    if (c == '\n') {
      if (*width == -1) {
        *width = i;
      }
      continue;
    }
    res[j++] = c;
  }
  *height = j / *width;
  return res;
}

static struct point *find_galaxies(const char *universe, int width, int height, int *ngalaxies) {
  int len = 0;
  int cap = 8;
  struct point *list = malloc(sizeof(*list) * cap);

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      if (len >= cap) {
        cap *= 2;
        list = realloc(list, sizeof(*list) * cap);
      }
      char c = IDX(universe, i, j, width);
      if (c == '#') {
        list[len].x = i;
        list[len].y = j;
        len++;
      }
    }
  }

  *ngalaxies = len;
  return list;
}

static int distance(struct point a, struct point b) { return ABS(a.x - b.x) + ABS(a.y - b.y); }

static struct point expand(struct point orig, const int *empty_columns, int ncols, const int *empty_rows, int nrows,
                           int times) {
  int add_x = 0;
  for (int i = 0; i < ncols; i++) {
    int x = empty_columns[i];
    if (orig.x > x) {
      add_x++;
    }
  }
  int add_y = 0;
  for (int i = 0; i < nrows; i++) {
    int y = empty_rows[i];
    if (orig.y > y) {
      add_y++;
    }
  }
  orig.x += add_x * (times - 1);
  orig.y += add_y * (times - 1);
  return orig;
}

static int *find_empty(const char *arr, int dim1, int dim2, int idxdim, int *length) {
  int cap = 4;
  int len = 0;
  int *list = malloc(sizeof(*list) * cap);

  for (int i = 0; i < dim1; i++) {
    bool empty = true;
    for (int j = 0; j < dim2; j++) {
      int x, y, width;
      if (idxdim == 1) {
        x = i;
        y = j;
        width = dim1;
      } else {
        x = j;
        y = i;
        width = dim2;
      }
      if (IDX(arr, x, y, width) == '#') {
        empty = false;
        break;
      }
    }

    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    if (empty) {
      list[len++] = i;
    }
  }

  *length = len;
  return list;
}

static int *find_empty_columns(const char *universe, int width, int height, int *length) {
  return find_empty(universe, width, height, 1, length);
}

static int *find_empty_rows(const char *universe, int width, int height, int *length) {
  return find_empty(universe, height, width, 2, length);
}

static void solution(const char *const input, char *const output, int times) {
  int width, height;
  char *universe = parse_input(input, &width, &height);

#ifdef DEBUG
  DBG("original");
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      fputc(IDX(universe, i, j, width), stderr);
    }
    fputc('\n', stderr);
  }
#endif

  int ngalaxies, ncolumns, nrows;
  struct point *galaxies = find_galaxies(universe, width, height, &ngalaxies);
  int *empty_columns = find_empty_columns(universe, width, height, &ncolumns);
  int *empty_rows = find_empty_rows(universe, width, height, &nrows);

#ifdef DEBUG
  DBG("expanded");
  {
    int *curr_empty_row = empty_rows;
    int *curr_empty_col = empty_columns;
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        fputc(IDX(universe, i, j, width), stderr);
        if (i == *curr_empty_col) {
          curr_empty_col++;
          fputc(IDX(universe, i, j, width), stderr);
        }
      }
      fputc('\n', stderr);
      curr_empty_col = empty_columns;

      if (j == *curr_empty_row) {
        curr_empty_row++;
        for (int i = 0; i < width; i++) {
          fputc(IDX(universe, i, j, width), stderr);
          if (i == *curr_empty_col) {
            curr_empty_col++;
            fputc(IDX(universe, i, j, width), stderr);
          }
        }
        fputc('\n', stderr);
        curr_empty_col = empty_columns;
      }
    }
  }
#endif

  long res = 0;
  for (int i = 0; i < ngalaxies; i++) {
    struct point galaxy1 = expand(galaxies[i], empty_columns, ncolumns, empty_rows, nrows, times);
    for (int j = i + 1; j < ngalaxies; j++) {
      struct point galaxy2 = expand(galaxies[j], empty_columns, ncolumns, empty_rows, nrows, times);

      int d = distance(galaxy1, galaxy2);
      // DBG("Distance between %d,%d and %d,%d is %d", galaxy1.x, galaxy1.y, galaxy2.x, galaxy2.y, d);
      res += d;
    }
  }

  free(universe);
  free(galaxies);
  free(empty_columns);
  free(empty_rows);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
}

static void solution1(const char *const input, char *const output) { solution(input, output, 2); }

static void solution2(const char *const input, char *const output) { solution(input, output, 1000000); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
