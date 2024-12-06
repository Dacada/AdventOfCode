#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ABS(x) (((x) < 0) ? (-(x)) : ((x)))
#define MAX(a, b) (((a) > (b)) ? ((a)) : ((b)))

static int cmp_int(const void *v1, const void *v2) {
  const int *p1 = v1;
  const int *p2 = v2;
  return *p1 - *p2;
}

static int parse_int(const char **input) {
  int n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    (*input)++;
  }
  return n;
}

static int parse_input(const char *input, int **l1, int **l2) {
  int len = 0;
  int cap = 4;
  *l1 = malloc(sizeof(**l1) * cap);
  *l2 = malloc(sizeof(**l2) * cap);

  while (*input != '\0') {
    ASSERT(isdigit(*input), "parse error");
    (*l1)[len] = parse_int(&input);
    while (isspace(*input)) {
      input++;
    }

    ASSERT(isdigit(*input), "parse error");
    (*l2)[len] = parse_int(&input);
    while (*input != '\0' && *input != '\n' && isspace(*input)) {
      input++;
    }

    len++;
    if (len >= cap) {
      cap *= 2;
      *l1 = realloc(*l1, sizeof(**l1) * cap);
      *l2 = realloc(*l2, sizeof(**l2) * cap);
    }

    if (*input == '\0') {
      break;
    }
    ASSERT(*input == '\n', "parse error");
    input++;
  }

  qsort(*l1, len, sizeof(**l1), cmp_int);
  qsort(*l2, len, sizeof(**l2), cmp_int);

  return len;
}

static void solution1(const char *const input, char *const output) {
  int *l1 = NULL;
  int *l2 = NULL;
  int len;
  len = parse_input(input, &l1, &l2);

  int total = 0;
  for (int i = 0; i < len; i++) {
    int n = ABS(l1[i] - l2[i]);
    DBG("%d - %d = %d", l1[i], l2[i], n);
    total += n;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(l1);
  free(l2);
}

static void solution2(const char *const input, char *const output) {
  int *l1 = NULL;
  int *l2 = NULL;
  int len;
  len = parse_input(input, &l1, &l2);

  int max_number = MAX(l1[len - 1], l2[len - 2]);
  int *l2_count = malloc(sizeof(*l2_count) * (max_number + 1));
  memset(l2_count, 0, sizeof(*l2_count) * (max_number + 1));
  for (int i = 0; i < len; i++) {
    l2_count[l2[i]]++;
  }

  int total = 0;
  for (int i = 0; i < len; i++) {
    total += l1[i] * l2_count[l1[i]];
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(l1);
  free(l2);
  free(l2_count);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
