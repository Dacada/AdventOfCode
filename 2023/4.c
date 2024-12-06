#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct card {
  int nnums;
  int nwins;
  int *nums;
  int *wins;
};

static void assert_str(const char **input, const char *exp) {
  int n = strlen(exp);
  ASSERT(strncmp(*input, exp, n) == 0, "parse error");
  *input += n;
}

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error '%d' '%c'", **input, **input);

  int n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static void skip_blank(const char **input) {
  while (isblank(**input)) {
    *input += 1;
  }
}

static int *parse_int_list(const char **input, int *len) {
  int l = 0;
  int c = 4;
  int *list = malloc(sizeof(*list) * c);

  while (isdigit(**input)) {
    if (l >= c) {
      c *= 2;
      list = realloc(list, sizeof(*list) * c);
    }
    list[l++] = parse_int(input);
    skip_blank(input);
  }

  *len = l;
  return list;
}

static void parse_line(const char **input, struct card *card) {
  assert_str(input, "Card");
  skip_blank(input);
  parse_int(input);
  assert_str(input, ":");
  skip_blank(input);

  card->wins = parse_int_list(input, &card->nwins);
  assert_str(input, "|");
  skip_blank(input);
  card->nums = parse_int_list(input, &card->nnums);
  if (**input == '\n') {
    *input += 1;
  }
}

static struct card *parse_input(const char *input, int *ncards) {
  int len = 0;
  int cap = 4;
  struct card *list = malloc(sizeof(*list) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    parse_line(&input, list + len);
    len++;
  }

  *ncards = len;
  return list;
}

static int get_score(const struct card *card) {
  int res = 0;
  for (int i = 0; i < card->nnums; i++) {
    for (int j = 0; j < card->nwins; j++) {
      if (card->nums[i] == card->wins[j]) {
        res++;
      }
    }
  }
  return res;
}

static void solution1(const char *const input, char *const output) {
  int ncards;
  struct card *cards = parse_input(input, &ncards);

  int res = 0;
  for (int i = 0; i < ncards; i++) {
    int n = get_score(cards + i);
    if (n > 0) {
      res += 1 << (n - 1);
    }
  }

  for (int i = 0; i < ncards; i++) {
    free(cards[i].nums);
    free(cards[i].wins);
  }
  free(cards);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

static void solution2(const char *const input, char *const output) {
  int ncards;
  struct card *cards = parse_input(input, &ncards);

  int *counts = malloc(sizeof(*counts) * ncards);
  for (int i = 0; i < ncards; i++) {
    counts[i] = 1;
  }

  for (int i = 0; i < ncards; i++) {
    int n = get_score(cards + i);
    for (int j = 0; j < n; j++) {
      counts[i + 1 + j] += counts[i];
    }
  }

  int res = 0;
  for (int i = 0; i < ncards; i++) {
    res += counts[i];
  }

  for (int i = 0; i < ncards; i++) {
    free(cards[i].nums);
    free(cards[i].wins);
  }
  free(cards);
  free(counts);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
