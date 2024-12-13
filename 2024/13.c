#include <aoclib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

struct point {
  long x, y;
};

struct machine {
  struct point v1;
  struct point v2;
  struct point v3;
};

static void parse_machine(const char **input, struct machine *machine) {
  const char *txt = "Button A: X+";
  aoc_expect_text(input, txt, strlen(txt));
  machine->v1.x = aoc_parse_long(input);
  txt = ", Y+";
  aoc_expect_text(input, txt, strlen(txt));
  machine->v1.y = aoc_parse_long(input);

  txt = "\nButton B: X+";
  aoc_expect_text(input, txt, strlen(txt));
  machine->v2.x = aoc_parse_long(input);
  txt = ", Y+";
  aoc_expect_text(input, txt, strlen(txt));
  machine->v2.y = aoc_parse_long(input);

  txt = "\nPrize: X=";
  aoc_expect_text(input, txt, strlen(txt));
  machine->v3.x = aoc_parse_long(input);
  txt = ", Y=";
  aoc_expect_text(input, txt, strlen(txt));
  machine->v3.y = aoc_parse_long(input);
}

static struct machine *parse_input(const char *input, long *len) {
  struct aoc_dynarr machines;
  aoc_dynarr_init(&machines, sizeof(struct machine), 8);

  while (*input != 0) {
    parse_machine(&input, aoc_dynarr_grow(&machines, 1));
    while (*input == '\n') {
      input++;
    }
  }

  *len = machines.len;
  return machines.data;
}

static void solution1(const char *input, char *const output) {
  long nmachines;
  struct machine *machines = parse_input(input, &nmachines);

  long total = 0;
  for (long i = 0; i < nmachines; i++) {
    struct machine machine = machines[i];

    long a, b;
    bool done = false;
    for (a = 0; a < 100; a++) {
      for (b = 0; b < 100; b++) {
        if (machine.v1.x * a + machine.v2.x * b == machine.v3.x &&
            machine.v1.y * a + machine.v2.y * b == machine.v3.y) {
          done = true;
          break;
        }
      }
      if (done) {
        break;
      }
    }

    if (done) {
      total += 3 * a + b;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
  free(machines);
}

static void solution2(const char *input, char *const output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
