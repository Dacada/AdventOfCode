#include <aoclib.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct vec2 {
  int x;
  int y;
};

struct segment {
  struct vec2 from;
  struct vec2 to;
};

static int manhattan_distance(struct vec2 p1, struct vec2 p2) { return ABS(p1.x - p2.x) + ABS(p1.y - p2.y); }

static int manhattan_distance_from_origin(struct vec2 point) {
  const struct vec2 origin = {.x = 0, .y = 0};
  return manhattan_distance(point, origin);
}

static bool line_segments_intersect(const struct segment *const line1, const struct segment *const line2,
                                    struct vec2 *const intersection) {
  // Detect when they are parallel
  bool horizontal1 = line1->from.y == line1->to.y;
  bool horizontal2 = line2->from.y == line2->to.y;
  if (horizontal1 && horizontal2) {
    return false;
  } else if (!horizontal1 && !horizontal2) {
    return false;
  }

  // Now we have two options left: Either line1 is horizontal
  // and line2 is vertical or vice versa.
  //
  // For them to cross:
  //
  //   * The two equal y coordinates of the horizontal line have
  //     to be between the two different y coordinates of the
  //     vertical one:
  //
  //     . . . . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . | . --------- .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . . . . . . . . .
  //
  //   * And the two equal x coordinates of the vertical line
  //     have to be between the two different x coodrinates of
  //     the horizontal one:
  //
  //     . . . . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . --X-------- . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . | . . . . . . .
  //     . . . . . . . . . .

  const struct segment *const vertical_line = horizontal1 ? line2 : line1;
  const struct segment *const horizontal_line = horizontal2 ? line2 : line1;

  int xver, xhor1, xhor2, yhor, yver1, yver2;

  xver = vertical_line->from.x;
  yhor = horizontal_line->from.y;

  if (horizontal_line->from.x < horizontal_line->to.x) {
    xhor1 = horizontal_line->from.x;
    xhor2 = horizontal_line->to.x;
  } else {
    xhor1 = horizontal_line->to.x;
    xhor2 = horizontal_line->from.x;
  }

  if (vertical_line->from.y < vertical_line->to.y) {
    yver1 = vertical_line->from.y;
    yver2 = vertical_line->to.y;
  } else {
    yver1 = vertical_line->to.y;
    yver2 = vertical_line->from.y;
  }

  if (yhor >= yver1 && yhor <= yver2 && xver >= xhor1 && xver <= xhor2) {
    intersection->x = xver;
    intersection->y = yhor;
    return true;
  } else {
    return false;
  }
}

static void vec2_mult_scalar(struct vec2 src, int scl, struct vec2 *dst) {
  dst->x = src.x * scl;
  dst->y = src.y * scl;
}

static void vec2_sum(struct vec2 src, struct vec2 *dst) {
  dst->x += src.x;
  dst->y += src.y;
}

static void add_line(struct segment *const buff, struct vec2 *const loc, struct vec2 mult, int num) {
  buff->from = *loc;
  struct vec2 tmp;
  vec2_mult_scalar(mult, num, &tmp);
  vec2_sum(tmp, loc);
  buff->to = *loc;
}

static struct segment *parse_circuit(const char *const input, size_t *const size, size_t *const i) {
  struct segment *lines = malloc(sizeof(*lines) * 32);
  size_t lines_size = 32;
  size_t lines_i = 0;

  struct vec2 location = {.x = 0, .y = 0};
  struct vec2 mult = {.x = 0, .y = 0};
  int num = 0;

  size_t j;
  for (j = 0; j < 1 << 16; j++) {
    char c = input[*i + j];

    if (c == ',') {
      ASSERT(mult.x != 0 || mult.y != 0, "Mult has not been set.");
      add_line(lines + lines_i, &location, mult, num);
      num = 0;
      lines_i++;
      while (lines_i >= lines_size) {
        lines_size *= 2;
        lines = realloc(lines, lines_size * sizeof(*lines));
      }
    } else if (c == 'U') {
      mult.x = 0;
      mult.y = 1;
    } else if (c == 'D') {
      mult.x = 0;
      mult.y = -1;
    } else if (c == 'R') {
      mult.x = 1;
      mult.y = 0;
    } else if (c == 'L') {
      mult.x = -1;
      mult.y = 0;
    } else if (c >= '0' && c <= '9') {
      num = num * 10 + c - '0';
    } else {
      add_line(lines + lines_i, &location, mult, num);
      break;
    }
  }

  *i += j + 1;
  *size = lines_i + 1;
  return lines;
}

static void solution1(const char *const input, char *const output) {
  size_t circuit1_size, circuit2_size;
  struct segment *circuit1, *circuit2;

  size_t k = 0;
  circuit1 = parse_circuit(input, &circuit1_size, &k);
  circuit2 = parse_circuit(input, &circuit2_size, &k);

  int best_distance = INT_MAX;
  for (size_t i = 0; i < circuit1_size; i++) {
    for (size_t j = 0; j < circuit2_size; j++) {
      struct segment *line1 = circuit1 + i;
      struct segment *line2 = circuit2 + j;
      struct vec2 intersection;
      if (line_segments_intersect(line1, line2, &intersection)) {
        int this_distance = manhattan_distance_from_origin(intersection);
        if (this_distance < best_distance && this_distance > 0) {
          best_distance = this_distance;
        }
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", best_distance);
  free(circuit1);
  free(circuit2);
}

static void solution2(const char *const input, char *const output) {
  size_t circuit1_size, circuit2_size;
  struct segment *circuit1, *circuit2;

  size_t k = 0;
  circuit1 = parse_circuit(input, &circuit1_size, &k);
  circuit2 = parse_circuit(input, &circuit2_size, &k);

  int best_distance = INT_MAX;
  int dist_i = 0;
  for (size_t i = 0; i < circuit1_size; i++) {
    struct segment *line1 = circuit1 + i;
    dist_i += manhattan_distance(line1->from, line1->to);

    int dist_j = 0;
    for (size_t j = 0; j < circuit2_size; j++) {
      struct segment *line2 = circuit2 + j;
      dist_j += manhattan_distance(line2->from, line2->to);

      struct vec2 intersection;
      if (line_segments_intersect(line1, line2, &intersection)) {
        int this_distance = dist_i + dist_j;
        this_distance -= manhattan_distance(intersection, line1->to);
        this_distance -= manhattan_distance(intersection, line2->to);
        if (this_distance < best_distance && this_distance > 0) {
          best_distance = this_distance;
        }
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", best_distance);
  free(circuit1);
  free(circuit2);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
