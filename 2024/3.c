#include <aoclib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void solution(const char *const input, char *const output,
                     bool can_disable) {
  const char *expected = "mul(NNN,MMM)";
  const int final_state = strlen(expected);
  const int end_n = 7;
  const int end_m = 11;
  int state = 0;
  int n = 0;
  int m = 0;
  int total = 0;
  bool disabled = false;

  for (int i = 0; input[i] != 0; i++) {
    char c = input[i];

    // too lazy to make this nice
    if (can_disable) {
      if (c == 'd') {
        state = 101;
        continue;
      }
      if (c == 'o' && state == 101) {
        state = 102;
        continue;
      }
      if (c == '(' && state == 102) {
        state = 201;
        continue;
      }
      if (c == ')' && state == 201) {
        disabled = false;
        state = 0;
        continue;
      }
      if (c == 'n' && state == 102) {
        state = 301;
        continue;
      }
      if (c == '\'' && state == 301) {
        state = 302;
        continue;
      }
      if (c == 't' && state == 302) {
        state = 303;
        continue;
      }
      if (c == '(' && state == 303) {
        state = 304;
        continue;
      }
      if (c == ')' && state == 304) {
        disabled = true;
        state = 0;
        continue;
      }
    }

    if (disabled) {
      continue;
    }

    char e = expected[state];
    if (e == 'N' || e == 'M') {
      int *num;
      if (e == 'N') {
        num = &n;
      } else {
        num = &m;
      }
      if (isdigit(c)) {
        DBG("digit: %c", c);
        *num *= 10;
        *num += c - '0';
        state++;
      } else {
        // Note that something like mul(,123) will be recognized as 0 * 123
        // which works for the problem as stated. Same for the other side.
        if (e == 'N') {
          DBG("int 1: %d", n);
          state = end_n;
        } else {
          DBG("int 2: %d", m);
          state = end_m;
        }
        i -= 1;
      }
    } else {
      if (c == e) {
        DBG("ok on %c", c);
        state++;
      } else {
        DBG("reset on %c", c);
        state = 0;
        n = 0;
        m = 0;
      }
      if (state == final_state) {
        state = 0;
        DBG("%d * %d", n, m);
        total += n * m;
        n = 0;
        m = 0;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

static void solution1(const char *const input, char *const output) {
  solution(input, output, false);
}

static void solution2(const char *const input, char *const output) {
  solution(input, output, true);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
