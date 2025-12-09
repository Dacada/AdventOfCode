#include <aoclib.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static bool parse_point(const char **input, void *ptr) {
  if (**input == '\0') {
    return false;
  }

  struct aoc_point *point = ptr;
  point->x = aoc_parse_int(input);
  aoc_expect_char(input, ',');
  point->y = aoc_parse_int(input);
  aoc_skip_space(input);
  return true;
}

static long calc_area(struct aoc_point a, struct aoc_point b) {
  long x = AOC_ABS(a.x - b.x) + 1;
  long y = AOC_ABS(a.y - b.y) + 1;
  return x * y;
}

static void solution1(const char *input, char *const output) {
  int npoints;
  struct aoc_point *points = aoc_parse_sequence(&input, &npoints, sizeof(*points), 64, parse_point);

  long max = 0;
  for (int i = 0; i < npoints; i++) {
    struct aoc_point a = points[i];
    for (int j = i + 1; j < npoints; j++) {
      struct aoc_point b = points[j];
      long area = calc_area(a, b);
      if (area > max) {
        max = area;
      }
    }
  }

  free(points);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", max);
}

// completely useless polygon abstraction but it lets me keep things tidy

struct edge {
  struct aoc_point a, b;
};

struct polygon {
  struct aoc_point *v;
  int nv;

  struct edge *e;
  int ne;
};

static bool in_range_normalize(int n, int a, int b) {
  int start = AOC_MIN(a, b);
  int end = AOC_MAX(a, b);
  return start <= n && n <= end;
}

static bool in_range_normalize_halfopen(int n, int a, int b) {
  int start = AOC_MIN(a, b);
  int end = AOC_MAX(a, b);
  return start < n && n <= end;
}

static bool in_range_normalize_open(int n, int a, int b) {
  int start = AOC_MIN(a, b);
  int end = AOC_MAX(a, b);
  return start < n && n < end;
}

static void polygon_init(struct polygon *poly, const struct aoc_point *points, int npoints) {
  poly->v = malloc(sizeof(*poly->v) * npoints);
  poly->nv = npoints;

  poly->e = malloc(sizeof(*poly->e) * npoints);
  poly->ne = npoints;

  for (int i = 0; i < npoints; i++) {
    poly->v[i].x = points[i].x;
    poly->v[i].y = points[i].y;
  }

  for (int i = 0; i < npoints; i++) {
    poly->e[i].a = poly->v[i];
    poly->e[i].b = poly->v[(i + 1) % poly->nv];
  }
}

static void polygon_free(struct polygon *poly) {
  free(poly->v);
  free(poly->e);
}

static bool point_inside_polygon(const struct polygon *poly, struct aoc_point p) {
  for (int i = 0; i < poly->ne; i++) {
    struct edge e = poly->e[i];

    // is it inside an edge?
    if (e.a.y == e.b.y) {
      if (p.y == e.a.y && in_range_normalize(p.x, e.a.x, e.b.x)) {
        return true;
      }
    } else if (e.a.x == e.b.x) {
      if (p.x == e.a.x && in_range_normalize(p.y, e.a.y, e.b.y)) {
        return true;
      }
    }
  }

  // ray casting in/out count
  int count = 0;
  for (int i = 0; i < poly->ne; i++) {
    struct edge e = poly->e[i];

    if (e.a.x == e.b.x) {
      if (e.a.x > p.x && in_range_normalize_halfopen(p.y, e.a.y, e.b.y)) {
        count += 1;
      }
    }
  }

  return count % 2 != 0;
}

static bool segment_intersects_polygon(const struct polygon *poly, struct edge r) {
  bool rect_vertical = (r.a.x == r.b.x);

  for (int i = 0; i < poly->ne; i++) {
    struct edge e = poly->e[i];
    bool poly_vertical = (e.a.x == e.b.x);

    // skip parallel lines
    if (rect_vertical == poly_vertical) {
      continue;
    }

    if (rect_vertical) {
      // rectangle vertical, polygon horizontal
      if (in_range_normalize_open(r.a.x, e.a.x, e.b.x) && in_range_normalize_open(e.a.y, r.a.y, r.b.y)) {
        return true;
      }
    } else {
      // rectangle horizontal, polygon vertical
      if (in_range_normalize_open(e.a.x, r.a.x, r.b.x) && in_range_normalize_open(r.a.y, e.a.y, e.b.y)) {
        return true;
      }
    }
  }

  return false;
}

static bool rectangle_in_polygon(struct aoc_point a, struct aoc_point b, const struct polygon *polygon) {
  struct aoc_point corners[4] = {
      {
          .x = AOC_MIN(a.x, b.x),
          .y = AOC_MIN(a.y, b.y),
      },
      {
          .x = AOC_MIN(a.x, b.x),
          .y = AOC_MAX(a.y, b.y),
      },
      {
          .x = AOC_MAX(a.x, b.x),
          .y = AOC_MAX(a.y, b.y),
      },
      {
          .x = AOC_MAX(a.x, b.x),
          .y = AOC_MIN(a.y, b.y),
      },
  };

  for (int i = 0; i < 4; i++) {
    struct aoc_point p = corners[i];
    if (p.x == a.x && p.y == a.y) {
      continue;
    }
    if (p.x == b.x && p.y == b.y) {
      continue;
    }
    if (!point_inside_polygon(polygon, p)) {
      return false;
    }
  }

  for (int i = 0; i < 4; i++) {
    struct edge segment = {
        .a = corners[i],
        .b = corners[(i + 1) % 4],
    };
    if (segment_intersects_polygon(polygon, segment)) {
      return false;
    }
  }

  return true;
}

static void solution2(const char *input, char *const output) {
  int npoints;
  struct aoc_point *points = aoc_parse_sequence(&input, &npoints, sizeof(*points), 64, parse_point);

  struct polygon polygon;
  polygon_init(&polygon, points, npoints);
  free(points);

  long max = 0;
  for (int i = 0; i < polygon.nv; i++) {
    struct aoc_point a = polygon.v[i];
    for (int j = i + 1; j < npoints; j++) {
      struct aoc_point b = polygon.v[j];
      if (!rectangle_in_polygon(a, b, &polygon)) {
        continue;
      }
      long area = calc_area(a, b);
      if (area > max) {
        max = area;
      }
    }
  }

  polygon_free(&polygon);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", max);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
