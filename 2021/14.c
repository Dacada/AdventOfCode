#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define ALPHALEN ('Z' - 'A' + 1)
#define PAIRSLEN (ALPHALEN * ALPHALEN)

#define TOPAIR(a, b) (((a) - 'A') * ALPHALEN + ((b) - 'A'))
#define FROMPAIR_1(a) (((a) / ALPHALEN) + 'A')
#define FROMPAIR_2(a) (((a) % ALPHALEN) + 'A')

static void parse_input_template(const char **const input, unsigned long pairs[PAIRSLEN]) {
  while (*(*input + 1) != '\n') {
    char a = **input;
    *input += 1;
    char b = **input;

    unsigned pair = TOPAIR(a, b);
    pairs[pair]++;
  }

  *input += 1;
  ASSERT(**input == '\n', "parse error");
  *input += 1;
  ASSERT(**input == '\n', "parse error");
  *input += 1;
}

static void parse_input_rules(const char **const input, char rules[PAIRSLEN]) {
  while (**input != '\0') {
    char a = **input;
    *input += 1;
    char b = **input;
    *input += 1;

    const char *const msg = " -> ";
    const size_t msglen = strlen(msg);
    ASSERT(strncmp(*input, msg, msglen) == 0, "parse error");
    *input += msglen;

    char c = **input;
    *input += 1;

    rules[TOPAIR(a, b)] = c;

    ASSERT(**input == '\n' || **input == '\0', "parse error");
    if (**input == '\n') {
      *input += 1;
    }
  }
}

static void apply_rules(const char rules[PAIRSLEN], unsigned long pairs[PAIRSLEN]) {
  static unsigned long newpairs[PAIRSLEN];
  memset(newpairs, 0, sizeof(newpairs));

  for (unsigned pair = 0; pair < PAIRSLEN; pair++) {
    if (pairs[pair] > 0) {
      char a = FROMPAIR_1(pair);
      char b = FROMPAIR_2(pair);
      char c = rules[pair];
      ASSERT(c != '\0', "unmatched rule");

      unsigned newpair1 = TOPAIR(a, c);
      unsigned newpair2 = TOPAIR(c, b);
      newpairs[newpair1] += pairs[pair];
      newpairs[newpair2] += pairs[pair];
    }
  }

  memcpy(pairs, newpairs, sizeof(newpairs));
}

static void solution(const char *input, char *const output, int steps) {
  static unsigned long pairs[PAIRSLEN];
  parse_input_template(&input, pairs);

  static char rules[PAIRSLEN];
  parse_input_rules(&input, rules);

  for (int i = 0; i < steps; i++) {
    apply_rules(rules, pairs);
  }

  static unsigned long letters[ALPHALEN];
  for (unsigned pair = 0; pair < PAIRSLEN; pair++) {
    if (pairs[pair] > 0) {
      char c1 = FROMPAIR_1(pair);
      char c2 = FROMPAIR_2(pair);

      letters[c1 - 'A'] += pairs[pair];
      letters[c2 - 'A'] += pairs[pair];
    }
  }

  unsigned long min = ULONG_MAX;
  unsigned long max = 0;
  for (unsigned i = 0; i < ALPHALEN; i++) {
    if (letters[i] > 0) {
      if (letters[i] % 2 != 0) {
        letters[i]++;
      }
      if (letters[i] < min) {
        min = letters[i];
      }
      if (letters[i] > max) {
        max = letters[i];
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", max / 2 - min / 2);
}

static void solution1(const char *const input, char *const output) { solution(input, output, 10); }

static void solution2(const char *const input, char *const output) { solution(input, output, 40); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
