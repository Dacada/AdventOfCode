#include <aoclib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void parse_input(const char *input, char **map, int *width, int *height, enum aoc_direction **movements,
                        int *nmovements, struct aoc_point *robot) {
  *map = aoc_parse_grid_chars(&input, height, width);

  for (int j = 0; j < *height; j++) {
    for (int i = 0; i < *width; i++) {
      char c = (*map)[AOC_2D_IDX(i, j, *width)];
      if (c == '@') {
        robot->x = i;
        robot->y = j;
      }
    }
  }

  struct aoc_dynarr directions;
  aoc_dynarr_init(&directions, sizeof(enum aoc_direction), 32);
  while (*input != '\0') {
    enum aoc_direction dir = 999;
    switch (*input) {
    case '^':
      dir = AOC_DIRECTION_NORTH;
      break;
    case '>':
      dir = AOC_DIRECTION_EAST;
      break;
    case 'v':
      dir = AOC_DIRECTION_SOUTH;
      break;
    case '<':
      dir = AOC_DIRECTION_WEST;
      break;
    case '\n':
      break;
    default:
      FAIL("bad direction: %c", *input);
    }
    if (dir != 999) {
      enum aoc_direction *d = aoc_dynarr_grow(&directions, 1);
      *d = dir;
    }
    input += 1;
  }
  *movements = directions.data;
  *nmovements = directions.len;
}

static bool advance(char *map, enum aoc_direction dir, struct aoc_point *current, int width, int height) {
  struct aoc_point next = aoc_point_move(*current, dir);
  char *next_c = &map[AOC_2D_IDX(next.x, next.y, width)];
  char *curr_c = &map[AOC_2D_IDX(current->x, current->y, width)];

  struct aoc_point n = next;
  switch (*next_c) {
  case '#':
    return false;
  case 'O':
    if (!advance(map, dir, &n, width, height)) {
      return false;
    }
    __attribute__((fallthrough));
  case '.':
    *next_c = *curr_c;
    *curr_c = '.';
    *current = next;
    return true;
  default:
    FAIL("unexpected next char: %c", *next_c);
  }
}

static void draw(const char *map, int width, int height) {
#ifdef DEBUG
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      char c = map[AOC_2D_IDX(i, j, width)];
      fputc(c, stderr);
    }
    fputc('\n', stderr);
  }
  fputc('\n', stderr);
#else
  (void)map;
  (void)width;
  (void)height;
#endif
}

static void solution1(const char *input, char *const output) {
  char *map;
  int width, height;
  enum aoc_direction *movements;
  int nmovements;
  struct aoc_point robot;
  parse_input(input, &map, &width, &height, &movements, &nmovements, &robot);

  DBG("start:");
  draw(map, width, height);

  for (int i = 0; i < nmovements; i++) {
    advance(map, movements[i], &robot, width, height);
  }

  DBG("end:");
  draw(map, width, height);

  int result = 0;
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      char c = map[AOC_2D_IDX(i, j, width)];
      if (c == 'O') {
        result += j * 100 + i;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
  free(map);
  free(movements);
}

static bool advance_box_ns(char *map, enum aoc_direction movement, struct aoc_point curr1, int width, int height) {
  struct aoc_point curr2 = curr1;
  curr2.x++;

  struct aoc_point next1 = aoc_point_move(curr1, movement);
  struct aoc_point next2 = aoc_point_move(curr2, movement);

  char *curr1_c = &map[AOC_2D_IDX(curr1.x, curr1.y, width)];
  char *curr2_c = &map[AOC_2D_IDX(curr2.x, curr2.y, width)];

  char *next1_c = &map[AOC_2D_IDX(next1.x, next1.y, width)];
  char *next2_c = &map[AOC_2D_IDX(next2.x, next2.y, width)];

  if (*next1_c == '#' || *next2_c == '#') {
    return false;
  }

  bool needs_undo = false;
  char *dupmap = aoc_malloc(sizeof(char) * width * height);
  memcpy(dupmap, map, sizeof(char) * width * height);

  if (*next1_c == '[') {
    if (!advance_box_ns(map, movement, next1, width, height)) {
      free(dupmap);
      return false;
    }
    needs_undo = true;
  }

  if (*next1_c == ']') {
    struct aoc_point p = next1;
    p.x--;
    if (!advance_box_ns(map, movement, p, width, height)) {
      free(dupmap);
      return false;
    }
    needs_undo = true;
  }

  ASSERT(*next1_c == '.', "invalid next1_c %c", *next1_c);

  if (*next2_c == '[') {
    if (!advance_box_ns(map, movement, next2, width, height)) {
      if (needs_undo) {
        memcpy(map, dupmap, sizeof(char) * width * height);
      }
      free(dupmap);
      return false;
    }
  }

  if (*next2_c == ']') {
    struct aoc_point p = next2;
    p.x--;
    if (!advance_box_ns(map, movement, p, width, height)) {
      if (needs_undo) {
        memcpy(map, dupmap, sizeof(char) * width * height);
      }
      free(dupmap);
      return false;
    }
  }
  free(dupmap);

  ASSERT(*next2_c == '.', "invalid next2_c %c", *next2_c);

  *curr1_c = '.';
  *curr2_c = '.';
  *next1_c = '[';
  *next2_c = ']';
  return true;
}

static bool advance_box_e(char *map, struct aoc_point curr1, int width, int height) {
  struct aoc_point curr2 = curr1;
  curr2.x++;
  struct aoc_point next = curr2;
  next.x++;

  char *curr1_c = &map[AOC_2D_IDX(curr1.x, curr1.y, width)];
  char *curr2_c = &map[AOC_2D_IDX(curr2.x, curr2.y, width)];
  char *next_c = &map[AOC_2D_IDX(next.x, next.y, width)];

  ASSERT(*curr1_c == '[' && *curr2_c == ']', "bad box");

  if (*next_c == '#') {
    return false;
  }

  if (*next_c == '[') {
    if (!advance_box_e(map, next, width, height)) {
      return false;
    }
  }
  ASSERT(*next_c == '.', "logic error");

  *curr1_c = '.';
  *curr2_c = '[';
  *next_c = ']';
  return true;
}

static bool advance_box_w(char *map, struct aoc_point curr1, int width, int height) {
  struct aoc_point curr2 = curr1;
  curr2.x++;
  struct aoc_point next = curr1;
  next.x--;

  char *curr1_c = &map[AOC_2D_IDX(curr1.x, curr1.y, width)];
  char *curr2_c = &map[AOC_2D_IDX(curr2.x, curr2.y, width)];
  char *next_c = &map[AOC_2D_IDX(next.x, next.y, width)];

  ASSERT(*curr1_c == '[' && *curr2_c == ']', "bad box");

  if (*next_c == '#') {
    return false;
  }

  if (*next_c == ']') {
    struct aoc_point n = next;
    n.x--;
    if (!advance_box_w(map, n, width, height)) {
      return false;
    }
  }
  ASSERT(*next_c == '.', "logic error");

  *next_c = '[';
  *curr1_c = ']';
  *curr2_c = '.';
  return true;
}

static bool advance_box(char *map, enum aoc_direction movement, struct aoc_point curr, int width, int height) {
  if (movement == AOC_DIRECTION_NORTH || movement == AOC_DIRECTION_SOUTH) {
    return advance_box_ns(map, movement, curr, width, height);
  } else if (movement == AOC_DIRECTION_EAST) {
    return advance_box_e(map, curr, width, height);
  } else if (movement == AOC_DIRECTION_WEST) {
    return advance_box_w(map, curr, width, height);
  } else {
    FAIL("bad movement: %d", movement);
  }
}

static void advance_robot(char *map, enum aoc_direction movement, struct aoc_point *current, int width, int height) {
  struct aoc_point next = aoc_point_move(*current, movement);
  char *curr_c = &map[AOC_2D_IDX(current->x, current->y, width)];
  char *next_c = &map[AOC_2D_IDX(next.x, next.y, width)];

  if (*next_c == '#') {
    return;
  } else {
    if (*next_c != '.') {
      struct aoc_point next_to_move = next;
      if (*next_c == ']') {
        next_to_move.x--;
      }
      char next_c_to_move = map[AOC_2D_IDX(next_to_move.x, next_to_move.y, width)];
      ASSERT(next_c_to_move == '[', "unexpected char: %c", next_c_to_move);
      if (!advance_box(map, movement, next_to_move, width, height)) {
        return;
      }
    }
    ASSERT(*next_c == '.', "expected empty after successful box move, got %c", *next_c);
    *curr_c = '.';
    *next_c = '@';
    *current = next;
  }
}

static void solution2(const char *input, char *const output) {
  char *map;
  int width, height;
  enum aoc_direction *movements;
  int nmovements;
  struct aoc_point robot;
  parse_input(input, &map, &width, &height, &movements, &nmovements, &robot);

  char *newmap = malloc(sizeof(*newmap) * width * 2 * height);
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      char c = map[AOC_2D_IDX(i, j, width)];
      char *nc1 = &newmap[AOC_2D_IDX(i * 2, j, width * 2)];
      char *nc2 = &newmap[AOC_2D_IDX(i * 2 + 1, j, width * 2)];
      switch (c) {
      case '#':
        *nc1 = '#';
        *nc2 = '#';
        break;
      case 'O':
        *nc1 = '[';
        *nc2 = ']';
        break;
      case '.':
        *nc1 = '.';
        *nc2 = '.';
        break;
      case '@':
        *nc1 = '@';
        *nc2 = '.';
        break;
      }
    }
  }
  width *= 2;
  free(map);
  map = newmap;
  robot.x *= 2;
  ASSERT(map[AOC_2D_IDX(robot.x, robot.y, width)] == '@', "oops");

  DBG("start:");
  draw(map, width, height);

  for (int i = 0; i < nmovements; i++) {
    advance_robot(map, movements[i], &robot, width, height);
  }

  DBG("end:");
  draw(map, width, height);

  int result = 0;
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      char c = map[AOC_2D_IDX(i, j, width)];
      if (c == '[') {
        result += j * 100 + i;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
  free(map);
  free(movements);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
