#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static bool is_symbol(char c) { return !isdigit(c) && c != '.' && c != '\n' && c != '\0'; }

static bool is_valid(const char *const input, int i, int span, int width, int length, int *gear_i) {
  DBG("looking for neighboring symbols");

  *gear_i = -1;

  i--;
  DBG("to the left?");
  if (i >= 0 && is_symbol(input[i])) {
    if (input[i] == '*') {
      DBG("gear adjacent! %d", i);
      *gear_i = i;
    }
    DBG("yup");
    return true;
  }

  i -= width;
  DBG("to the up?");
  if (i >= 0) {
    for (int j = i; j < i + 1 + span + 1; j++) {
      if (j >= 0 && j < length && is_symbol(input[j])) {
        if (input[j] == '*') {
          DBG("gear adjacent! %d", j);
          *gear_i = j;
        }
        DBG("yup, at j=%d", j - i);
        return true;
      }
    }
  }
  i += width * 2;
  DBG("to the down?");
  if (i < length) {
    for (int j = i; j < i + 1 + span + 1; j++) {
      if (j >= 0 && j < length && is_symbol(input[j])) {
        if (input[j] == '*') {
          DBG("gear adjacent! %d", j);
          *gear_i = j;
        }
        DBG("yup, at j=%d", j - i);
        return true;
      }
    }
  }
  i -= width;

  i += span + 1;
  DBG("to the right?");
  if (i < length && is_symbol(input[i])) {
    if (input[i] == '*') {
      DBG("gear adjacent! %d", i);
      *gear_i = i;
    }
    DBG("yup");
    return true;
  }

  return false;
}

static int solution(const char *const input, bool second) {
  int length = strlen(input);
  int width = 0;
  for (int i = 0;; i++) {
    if (input[i] == '\n') {
      width = i + 1;
      break;
    }
  }

  int *gears_1 = malloc(sizeof(*gears_1) * length);
  int *gears_2 = malloc(sizeof(*gears_2) * length);
  int *gears_3 = malloc(sizeof(*gears_3) * length);

  memset(gears_1, 0, sizeof(*gears_1) * length);
  memset(gears_2, 0, sizeof(*gears_2) * length);
  for (int i = 0; i < length; i++) {
    gears_3[i] = 1;
  }

  int result = 0;
  for (int i = 0;; i++) {
    char c = input[i];

    if (c == '\n') {
      continue;
    }
    if (c == '\0') {
      break;
    }

    if (isdigit(c)) {
      int span = 0;
      int n = 0;
      while (isdigit(c)) {
        n *= 10;
        n += c - '0';
        c = input[++i];
        span++;
      }

      DBG("found number %d", n);
      int gear_i;
      if (is_valid(input, i - span, span, width, length, &gear_i)) {
        result += n;
      }

      if (gear_i >= 0) {
        if (gears_1[gear_i] == 0) {
          gears_1[gear_i] = n;
        } else if (gears_2[gear_i] == 0) {
          gears_2[gear_i] = n;
        } else {
          gears_3[gear_i] = 0;
        }
      }
    }
  }

  if (!second) {
    goto end;
  }

  result = 0;
  for (int i = 0; i < length; i++) {
    result += gears_1[i] * gears_2[i] * gears_3[i];
  }

end:
  free(gears_1);
  free(gears_2);
  free(gears_3);
  return result;
}

static void solution1(const char *const input, char *const output) {
  int result = solution(input, false);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *const input, char *const output) {
  int result = solution(input, true);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
