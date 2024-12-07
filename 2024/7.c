#include <aoclib.h>
#include <stdio.h>

struct equation {
  long result;
  int *operands;
  int noperands;
};

bool with_concat = false;

static struct equation parse_line(const char **input) {
  struct equation eq;
  eq.result = aoc_parse_long(input);

  ASSERT(**input == ':', "parse error");
  *input += 1;

  struct aoc_dynarr operands;
  aoc_dynarr_init(&operands, sizeof(int), 4);

  while (**input != '\n' && **input != '\0') {
    ASSERT(**input == ' ', "parse error");
    *input += 1;

    int *next = aoc_dynarr_grow(&operands, 1);
    *next = aoc_parse_int(input);
  }

  if (**input == '\n') {
    *input += 1;
  }

  eq.operands = operands.data;
  eq.noperands = operands.len;
  return eq;
}

static struct aoc_dynarr parse_input(const char *input) {
  struct aoc_dynarr equations;
  aoc_dynarr_init(&equations, sizeof(struct equation), 4);

  while (*input != '\0') {
    struct equation *eq = aoc_dynarr_grow(&equations, 1);
    *eq = parse_line(&input);
  }

  return equations;
}

static int next_power_of_10(int n) {
  int powers_of_ten[] = {
      1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000,
  };

  for (size_t i = 0; i < sizeof(powers_of_ten) / sizeof(powers_of_ten[0]); i++) {
    int p = powers_of_ten[i];
    if (p > n) {
      return p;
    }
  }

  FAIL("%d too long for int?", n);
}

__attribute__((pure)) static bool is_solvable(const struct equation eq) {
#ifdef DEBUG
  fprintf(stderr, "%ld:", eq.result);
  for (int i = 0; i < eq.noperands; i++) {
    fprintf(stderr, " %d", eq.operands[i]);
  }
  fputc('\n', stderr);
#endif

  if (eq.noperands == 1) {
    return eq.operands[0] == eq.result;
  }

  struct equation new_eq;
  new_eq.operands = eq.operands;
  new_eq.noperands = eq.noperands - 1;
  int last = eq.operands[eq.noperands - 1];

  if (with_concat) {
    int n = next_power_of_10(last);
    if (eq.result % n == last) {
      new_eq.result = eq.result / n;
      if (is_solvable(new_eq)) {
        return true;
      }
    }
  }

  if (eq.result % last == 0) {
    new_eq.result = eq.result / last;
    if (is_solvable(new_eq)) {
      return true;
    }
  }

  if (eq.result >= last) {
    new_eq.result = eq.result - last;
    if (is_solvable(new_eq)) {
      return true;
    }
  }

  return false;
}

static void solution(const char *const input, char *const output) {
  struct aoc_dynarr equations = parse_input(input);
  struct equation *arr = equations.data;

  long result = 0;
  for (int i = 0; i < equations.len; i++) {
    struct equation eq = arr[i];
    if (is_solvable(eq)) {
      result += eq.result;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
  for (int i = 0; i < equations.len; i++) {
    free(arr[i].operands);
  }
  aoc_dynarr_free(&equations);
}

static void solution1(const char *const input, char *const output) {
  with_concat = false;
  solution(input, output);
}

static void solution2(const char *const input, char *const output) {
  with_concat = true;
  solution(input, output);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
