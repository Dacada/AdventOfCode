#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>

struct point {
  long x;
  long y;
};

struct instruction {
  char dir;
  long dist;
};

static long parse_hex_digit(char c) {
  ASSERT(isxdigit(c), "parse error c=%c(%d)", c, c);
  if (isdigit(c)) {
    return c - '0';
  } else if (c >= 'a') {
    return c - 'a' + 10;
  } else if (c >= 'A') {
    return c - 'A' + 10;
  } else {
    FAIL("parse error");
  }
}

static long parse_long(const char **input) {
  ASSERT(isdigit(**input), "parse error");
  long n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static void parse_instruction_1(const char **input, struct instruction *instruction) {
  instruction->dir = **input;
  *input += 1;

  ASSERT(**input == ' ', "parse error");
  *input += 1;

  instruction->dist = parse_long(input);

  ASSERT(**input == ' ', "parse error");
  *input += 1;

  ASSERT(**input == '(', "parse error");
  while (**input != ')') {
    *input += 1;
  }
  *input += 1;

  if (**input == '\n') {
    *input += 1;
  }
}

static void parse_instruction_2(const char **input, struct instruction *instruction) {
  while (**input != '(') {
    *input += 1;
  }
  *input += 1;
  ASSERT(**input == '#', "parse error");
  *input += 1;

  long n = 0;
  for (int i = 0; i < 5; i++) {
    n *= 16;
    n += parse_hex_digit(**input);
    *input += 1;
  }
  instruction->dist = n;

  switch (**input) {
  case '0':
    instruction->dir = 'R';
    break;
  case '1':
    instruction->dir = 'D';
    break;
  case '2':
    instruction->dir = 'L';
    break;
  case '3':
    instruction->dir = 'U';
    break;
  default:
    FAIL("invalid direction %c", **input);
  }
  *input += 1;

  ASSERT(**input == ')', "parse error c=%c(%d)", **input, **input);
  *input += 1;

  if (**input == '\n') {
    *input += 1;
  }
}

static struct instruction *parse_input(const char *input, int *length,
                                       void (*parse_instruction)(const char **, struct instruction *)) {
  int len = 0;
  int cap = 8;
  struct instruction *list = malloc(sizeof(*list) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    parse_instruction(&input, &list[len++]);
  }

  *length = len;
  return list;
}

static void solution(const char *const input, char *const output,
                     void (*parse_instruction)(const char **, struct instruction *)) {
  int len;
  struct instruction *instructions = parse_input(input, &len, parse_instruction);

#ifdef DEBUG
  for (int i = 0; i < len; i++) {
    fprintf(stderr, "%c %ld\n", instructions[i].dir, instructions[i].dist);
  }
#endif

  struct point p = {0, 0};
  long perimeter = 0;
  struct point *points = malloc(sizeof(*points) * (len + 1));
  points[0] = p;
  for (int i = 0; i < len; i++) {
    perimeter += instructions[i].dist;
    switch (instructions[i].dir) {
    case 'U':
      p.y -= instructions[i].dist;
      break;
    case 'L':
      p.x -= instructions[i].dist;
      break;
    case 'D':
      p.y += instructions[i].dist;
      break;
    case 'R':
      p.x += instructions[i].dist;
      break;
    }
    points[i + 1] = p;
  }
  ASSERT(perimeter % 2 == 0, "bad area?");

  long shoelace_area = 0;
  for (int i = 0; i < len; i++) {
    struct point p1 = points[i];
    struct point p2 = points[i + 1];
    shoelace_area += p1.x * p2.y - p2.x * p1.y;
  }
  ASSERT(shoelace_area % 2 == 0, "bad area?");
  shoelace_area /= 2;

  long res = shoelace_area + (perimeter / 2) + 1;

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
  free(instructions);
  free(points);
}

static void solution1(const char *const input, char *const output) { solution(input, output, parse_instruction_1); }

static void solution2(const char *const input, char *const output) { solution(input, output, parse_instruction_2); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
