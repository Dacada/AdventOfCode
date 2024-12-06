#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct bingo {
  unsigned numbers[5][5];
  bool marked[5][5];
};

static unsigned parse_number(const char **const input) {
  unsigned n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static unsigned *parse_calls(const char **const input, size_t *const len) {
  size_t size = 16;
  *len = 0;
  unsigned *list = malloc(size * sizeof(*list));

  do {
    if (*len >= size) {
      size *= 2;
      list = realloc(list, size * sizeof(*list));
    }
    list[*len] = parse_number(input);
    *len += 1;
  } while (*((*input)++) == ',');

  ASSERT(**input == '\n', "parse error");
  *input += 1;

  return list;
}

static void parse_bingo(const char **const input, struct bingo *const b) {
  for (unsigned i = 0; i < 5; i++) {
    for (unsigned j = 0; j < 5; j++) {
      while (isspace(**input)) {
        *input += 1;
      }
      b->numbers[i][j] = parse_number(input);
      b->marked[i][j] = false;
      while (isspace(**input)) {
        *input += 1;
      }
    }
  }
}

static struct bingo *parse_bingos(const char **const input, size_t *const len) {
  size_t size = 16;
  *len = 0;
  struct bingo *list = malloc(size * sizeof(*list));

  while (**input != '\0') {
    if (*len >= size) {
      size *= 2;
      list = realloc(list, size * sizeof(*list));
    }
    parse_bingo(input, &list[*len]);
    *len += 1;
  }

  return list;
}

static void parse_input(const char *input, struct bingo **const bingos, unsigned **const calls, size_t *const nbingos,
                        size_t *const ncalls) {
  *calls = parse_calls(&input, ncalls);
  *bingos = parse_bingos(&input, nbingos);
}

static bool process_call(struct bingo *const b, unsigned call) {
  bool vlines[5];
  for (int x = 0; x < 5; x++) {
    vlines[x] = true;
  }

  for (int y = 0; y < 5; y++) {
    bool hline = true;
    for (int x = 0; x < 5; x++) {
      if (b->numbers[y][x] == call) {
        b->marked[y][x] = true;
      }
      hline &= b->marked[y][x];
      vlines[x] &= b->marked[y][x];
    }
    if (hline) {
      return true;
    }
  }

  for (int x = 0; x < 5; x++) {
    if (vlines[x]) {
      return true;
    }
  }

  return false;
}

static unsigned score(struct bingo *const b, unsigned call) {
  unsigned result = 0;
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      if (!b->marked[y][x]) {
        result += b->numbers[y][x];
      }
    }
  }
  return result * call;
}

static void solution1(const char *const input, char *const output) {
  size_t nbingos, ncalls;
  struct bingo *bingos;
  unsigned *calls;
  parse_input(input, &bingos, &calls, &nbingos, &ncalls);

  size_t winner = 0;
  unsigned winner_call = 0;
  for (size_t i = 0; i < ncalls; i++) {
    for (size_t j = 0; j < nbingos; j++) {
      if (process_call(bingos + j, calls[i])) {
        winner = j;
        winner_call = calls[i];
        goto end;
      }
    }
  }

end:
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", score(bingos + winner, winner_call));
  free(bingos);
  free(calls);
}

static void solution2(const char *const input, char *const output) {
  size_t nbingos, ncalls;
  struct bingo *bingos;
  unsigned *calls;
  parse_input(input, &bingos, &calls, &nbingos, &ncalls);

  bool *winners = malloc(nbingos * sizeof(*winners));
  memset(winners, 0, nbingos * sizeof(*winners));
  unsigned total_winners = 0;

  size_t loser = 0;
  unsigned loser_call = 0;
  for (size_t i = 0; i < ncalls; i++) {
    for (size_t j = 0; j < nbingos; j++) {
      if (winners[j]) {
        continue;
      }

      if (process_call(bingos + j, calls[i])) {
        total_winners++;
        winners[j] = true;
        if (total_winners == nbingos) {
          loser = j;
          loser_call = calls[i];
          goto end;
        }
      }
    }
  }

end:;

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", score(bingos + loser, loser_call));
  free(bingos);
  free(calls);
  free(winners);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
