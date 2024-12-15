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

static void solution(const char *input, char *const output, bool more) {
  long nmachines;
  struct machine *machines = parse_input(input, &nmachines);

  long total = 0;
  for (long i = 0; i < nmachines; i++) {
    struct machine machine = machines[i];
    struct point v1 = machine.v1;
    struct point v2 = machine.v2;
    struct point v3 = machine.v3;

    if (more) {
      v3.x += 10000000000000L;
      v3.y += 10000000000000L;
    }

    long n_v2 = (v1.x * v3.y - v1.y * v3.x) / (v2.y * v1.x - v2.x * v1.y);
    long rem_v2 = (v1.x * v3.y - v1.y * v3.x) % (v2.y * v1.x - v2.x * v1.y);
    long n_v1 = (v3.x - n_v2 * v2.x) / v1.x;
    long rem_v1 = (v3.x - n_v2 * v2.x) % v1.x;
    if (rem_v2 != 0 || rem_v1 != 0) {
      continue;
    }
    if (!more && (n_v1 > 100 || n_v2 > 100)) {
      continue;
    }
    total += n_v1 * 3 + n_v2;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
  free(machines);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
