#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>

enum command {
  FORWARD,
  DOWN,
  UP,
};

struct step {
  enum command command;
  unsigned value;
};

static unsigned parse_number(const char **const input) {
  unsigned num = 0;
  while (isdigit(**input)) {
    num *= 10;
    num += **input - '0';
    *input += 1;
  }
  return num;
}

static struct step parse_line(const char **const input) {
  struct step r = {0};

  if (**input == 'f') {
    r.command = FORWARD;
    *input += 8;
  } else if (**input == 'd') {
    r.command = DOWN;
    *input += 5;
  } else if (**input == 'u') {
    r.command = UP;
    *input += 3;
  } else {
    ASSERT(false, "parse error");
  }

  r.value = parse_number(input);
  ASSERT(**input == '\n' || **input == '\0', "parse error");
  while (**input == '\n') {
    *input += 1;
  }

  return r;
}

static struct step *parse_input(const char *input, size_t *const len) {
  size_t size = 16;
  *len = 0;

  struct step *list = malloc(size * sizeof(*list));

  while (*input != '\0') {
    if (*len >= size) {
      size *= 2;
      list = realloc(list, size * sizeof(*list));
    }
    list[*len] = parse_line(&input);
    *len += 1;
  }

  return list;
}

static void solution1(const char *const input, char *const output) {
  size_t nsteps;
  struct step *steps = parse_input(input, &nsteps);

  int x = 0;
  int y = 0;
  for (size_t i = 0; i < nsteps; i++) {
    unsigned val = steps[i].value;
    switch (steps[i].command) {
    case FORWARD:
      x += val;
      break;
    case DOWN:
      y += val;
      break;
    case UP:
      y -= val;
      break;
    default:
      ASSERT(false, "oof");
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", x * y);
  free(steps);
}

static void solution2(const char *const input, char *const output) {
  size_t nsteps;
  struct step *steps = parse_input(input, &nsteps);

  int aim = 0;
  int x = 0;
  int y = 0;
  for (size_t i = 0; i < nsteps; i++) {
    unsigned val = steps[i].value;
    switch (steps[i].command) {
    case FORWARD:
      x += val;
      y += aim * val;
      break;
    case DOWN:
      aim += val;
      break;
    case UP:
      aim -= val;
      break;
    default:
      ASSERT(false, "oof");
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", x * y);
  free(steps);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
