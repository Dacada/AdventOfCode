#include <aoclib.h>
#include <stdio.h>
#include <stdlib.h>

#define ENCODE(str) (((str)[0] - 'a') * 26 * 26 + ((str)[1] - 'a') * 26 + (str)[2] - 'a')
#define DECODE(n, str)                                                                                                 \
  do {                                                                                                                 \
    str[2] = 'a' + n % 26;                                                                                             \
    str[1] = 'a' + (n / 26) % 26;                                                                                      \
    str[0] = 'a' + n / 26 / 26;                                                                                        \
  } while (0)

#define SPACE_LEN (ENCODE("zzz") + 1)

static int parse_int_26(const char **input) {
  for (int i = 0; i < 3; i++) {
    ASSERT('a' <= (*input)[i] && (*input)[i] <= 'z', "parse error");
  }

  int n = ENCODE(*input);
  *input += 3;
  return n;
}

struct adj {
  bool exists;
  int noutv;
  int *outv;
};

static bool parse_noutv_entry(const char **input, void *ptr) {
  if (**input == '\n') {
    *input += 1;
    return false;
  }

  int *n = ptr;
  *n = parse_int_26(input);
  if (**input != '\n') {
    aoc_expect_char(input, ' ');
  }
  return true;
}

static int parse_adj(const char **input, struct adj *adj) {
  if (**input == '\0') {
    return -1;
  }

  int idx = parse_int_26(input);
  aoc_expect_text(input, ": ", 2);

  adj->exists = true;
  adj->outv = aoc_parse_sequence(input, &adj->noutv, sizeof(*adj->outv), 4, parse_noutv_entry);
  return idx;
}

static struct adj *parse_input(const char **input) {
  struct adj *adjs = malloc(sizeof(*adjs) * SPACE_LEN);
  for (int i = 0; i < SPACE_LEN; i++) {
    adjs[i].exists = false;
    adjs[i].noutv = 0;
    adjs[i].outv = NULL;
  }

  struct adj adj;
  int idx;
  while ((idx = parse_adj(input, &adj)) >= 0) {
    ASSERT(!adjs[idx].exists, "parse error");
    adjs[idx] = adj;
  }

  return adjs;
}

static long count_paths_inner(const struct adj *adjs, int u, int t, long *memo) {
  if (memo[u] >= 0) {
    return memo[u];
  }

  if (u == t) {
    memo[u] = 1;
    return 1;
  }

  // implied terminal node for all paths in the dag in some inputs
  if (u == ENCODE("out")) {
    return 0;
  }

  struct adj from = adjs[u];
  {
#ifdef DEBUG
    char str[3];
    DECODE(u, str);
#endif
    ASSERT(from.exists, "from node %.3s does not exist", str);
  }

  long res = 0;
  for (int i = 0; i < from.noutv; i++) {
    res += count_paths_inner(adjs, from.outv[i], t, memo);
  }
  memo[u] = res;

  return res;
}

static long count_paths(const struct adj *adjs, int u, int t) {
  long *memo = malloc(sizeof(*memo) * SPACE_LEN);
  for (int i = 0; i < SPACE_LEN; i++) {
    memo[i] = -1;
  }
  long res = count_paths_inner(adjs, u, t, memo);
  free(memo);
  return res;
}

static void solution1(const char *input, char *const output) {
  struct adj *adjs = parse_input(&input);

  long res = count_paths(adjs, ENCODE("you"), ENCODE("out"));

  for (int i = 0; i < SPACE_LEN; i++) {
    free(adjs[i].outv);
  }
  free(adjs);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
}

static void solution2(const char *input, char *const output) {
  struct adj *adjs = parse_input(&input);

  // DAG factorization: all paths that go from a to b passing through c and d in order can be calculated as count(a to
  // c) * count(c to d) * count(d to b)

  long one_order = count_paths(adjs, ENCODE("svr"), ENCODE("dac")) * count_paths(adjs, ENCODE("dac"), ENCODE("fft")) *
                   count_paths(adjs, ENCODE("fft"), ENCODE("out"));

  long other_order = count_paths(adjs, ENCODE("svr"), ENCODE("fft")) * count_paths(adjs, ENCODE("fft"), ENCODE("dac")) *
                     count_paths(adjs, ENCODE("dac"), ENCODE("out"));

  long result = one_order + other_order;

  for (int i = 0; i < SPACE_LEN; i++) {
    free(adjs[i].outv);
  }
  free(adjs);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
