#include <aoclib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

struct report {
  int *levels;
  int nlevels;
  int cap;
};

static bool report_is_safe_all(struct report *rep, bool descending, int dampen, int start) {
  for (int i = start; i < rep->nlevels - 1; i++) {
    if (dampen == i) {
      continue;
    }
    int a = rep->levels[i];
    int b = rep->levels[i + 1];
    if (dampen == i + 1) {
      if (i + 2 >= rep->nlevels) {
        continue;
      }
      b = rep->levels[i + 2];
    }

    if (!descending) {
      int tmp = a;
      a = b;
      b = tmp;
    }

    if (a <= b) {
      return false;
    }
    if (a - b > 3) {
      return false;
    }
  }
  return true;
}

static bool report_is_safe(struct report *rep, int dampen) {
  if (rep->nlevels <= 1) {
    return false;
  }

  int a = rep->levels[0];
  int b = rep->levels[1];
  int start = 1;
  if (dampen == 0) {
    a = b;
    b = rep->levels[2];
    start = 2;
  } else if (dampen == 1) {
    b = rep->levels[2];
    start = 2;
  }

  bool descending;
  if (a > b) {
    if (a - b > 3) {
      return false;
    }
    descending = true;
  } else if (a < b) {
    if (b - a > 3) {
      return false;
    }
    descending = false;
  } else {
    return false;
  }

  return report_is_safe_all(rep, descending, dampen, start);
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

static struct report parse_line(const char **input) {
  struct report rep;
  rep.cap = 8;
  rep.nlevels = 0;
  rep.levels = malloc(sizeof(*rep.levels) * rep.cap);

  while (**input != '\n') {
    ASSERT(isdigit(**input), "parse error");
    if (rep.nlevels >= rep.cap) {
      rep.cap *= 2;
      rep.levels = realloc(rep.levels, sizeof(*rep.levels) * rep.cap);
    }
    rep.levels[rep.nlevels++] = parse_int(input);
    while (**input != '\n' && isspace(**input)) {
      *input += 1;
    }
  }

  return rep;
}

static int parse_input(const char *input, struct report **reports) {
  int len = 0;
  int cap = 8;
  *reports = malloc(sizeof(**reports) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      *reports = realloc(*reports, sizeof(**reports) * cap);
    }
    (*reports)[len++] = parse_line(&input);
    if (*input != '\0') {
      ASSERT(*input == '\n', "parse error");
      input += 1;
    }
  }

  return len;
}

static void solution1(const char *const input, char *const output) {
  struct report *reports;
  int nreports = parse_input(input, &reports);

#ifdef DEBUG
  for (int i = 0; i < nreports; i++) {
    for (int j = 0; j < reports[i].nlevels; j++) {
      fprintf(stderr, "%d ", reports[i].levels[j]);
    }
    fprintf(stderr, "\n");
  }
#endif

  int total = 0;
  for (int i = 0; i < nreports; i++) {
    if (report_is_safe(&reports[i], -1)) {
      total++;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  for (int i = 0; i < nreports; i++) {
    free(reports[i].levels);
  }
  free(reports);
}

static void solution2(const char *const input, char *const output) {
  struct report *reports;
  int nreports = parse_input(input, &reports);

  int total = 0;
  for (int i = 0; i < nreports; i++) {
    for (int j = -1; j < reports[i].nlevels; j++) {
      if (report_is_safe(&reports[i], j)) {
        DBG("report %d is safe by dampening level %d", i, j);
        total++;
        break;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  for (int i = 0; i < nreports; i++) {
    free(reports[i].levels);
  }
  free(reports);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
