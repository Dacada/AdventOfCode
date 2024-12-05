#include <aoclib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// during part 2 i realized: wait a second, this is all just about sorting

#define NUM_IDS 100
#define IDX(arr, i, j) ((arr)[(j) * NUM_IDS + (i)])

static int parse_2digit_int(const char **input) {
  int n = 0;
  ASSERT(isdigit(**input), "parse error");
  n += **input - '0';
  n *= 10;
  *input += 1;
  ASSERT(isdigit(**input), "parse error");
  n += **input - '0';
  *input += 1;
  ASSERT(n >= 10 && n <= 99, "parse error");
  return n;
}

static void parse_dependencies(const char **input, bool *dependencies) {
  while (**input != '\n') {
    int n1 = parse_2digit_int(input);
    ASSERT(**input == '|', "parse error");
    *input += 1;

    int n2 = parse_2digit_int(input);
    ASSERT(**input == '\n', "parse error");
    *input += 1;

    IDX(dependencies, n1, n2) = true;
  }
  *input += 1;
}

struct sequence {
  int *numbers;
  int len;
  int cap;
};

static struct sequence parse_sequence(const char **input) {
  struct sequence res;
  res.len = 0;
  res.cap = 8;
  res.numbers = malloc(sizeof(*res.numbers) * res.cap);
  for (;;) {
    int n = parse_2digit_int(input);
    if (res.len >= res.cap) {
      res.cap *= 2;
      res.numbers = realloc(res.numbers, sizeof(*res.numbers) * res.cap);
    }
    res.numbers[res.len++] = n;
    if (**input == '\0') {
      break;
    }
    if (**input == '\n') {
      *input += 1;
      break;
    }
    ASSERT(**input == ',', "parse error");
    *input += 1;
  }
  return res;
}

static struct sequence *parse_sequences(const char *input, int *nsequences) {
  int cap = 8;
  int len = 0;
  struct sequence *buff = malloc(sizeof(*buff) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      buff = realloc(buff, sizeof(*buff) * cap);
    }
    buff[len++] = parse_sequence(&input);
  }

  *nsequences = len;
  return buff;
}

static struct sequence *parse_input(const char *input, bool *dependencies,
                                    int *nsequences) {
  parse_dependencies(&input, dependencies);
  return parse_sequences(input, nsequences);
}

static int sequence_midpoint(const struct sequence *seq) {
  return seq->numbers[seq->len / 2];
}

static bool is_sequence_valid(const struct sequence *seq,
                              const bool *dependencies) {
  for (int i = 0; i < seq->len - 1; i++) {
    int a = seq->numbers[i];
    int b = seq->numbers[i + 1];
    if (IDX(dependencies, b, a)) {
      return false;
    }
  }
  return true;
}

// DFS topological sorting
static void visit(int i, const struct sequence *seq, bool *marked,
                  const bool *dependencies, int *new, int *inew) {
  if (marked[i]) {
    return;
  }

  for (int j = 0; j < seq->len; j++) {
    if (j == i) {
      continue;
    }
    if (!IDX(dependencies, seq->numbers[i], seq->numbers[j])) {
      continue;
    }
    visit(j, seq, marked, dependencies, new, inew);
  }

  marked[i] = true;
  new[(*inew)++] = seq->numbers[i];
}

static void make_sequence_valid(struct sequence *seq,
                                const bool *dependencies) {
  int *new = malloc(sizeof(*new) * seq->len);
  int inew = 0;
  bool marked[seq->len];
  memset(marked, 0, sizeof(*marked) * seq->len);

  while (true) {
    int i;
    bool found = false;
    for (i = 0; i < seq->len; i++) {
      if (!marked[i]) {
        found = true;
        break;
      }
    }
    if (!found) {
      break;
    }

    visit(i, seq, marked, dependencies, new, &inew);
  }

  free(seq->numbers);
  seq->numbers = new;
}

static void solution(const char *const input, char *const output,
                     bool correcting) {
  static bool dependencies[NUM_IDS * NUM_IDS];
  int nsequences;
  struct sequence *sequences = parse_input(input, dependencies, &nsequences);

#ifdef DEBUG
  for (int i = 0; i < nsequences; i++) {
    for (int j = 0; j < sequences[i].len; j++) {
      fprintf(stderr, "%d,", sequences[i].numbers[j]);
    }
    fprintf(stderr, "\n");
  }
#endif

  int result = 0;
  for (int i = 0; i < nsequences; i++) {
    struct sequence *seq = &sequences[i];
    if (is_sequence_valid(seq, dependencies)) {
      if (!correcting) {
        result += sequence_midpoint(seq);
      }
    } else if (correcting) {
      make_sequence_valid(seq, dependencies);
      result += sequence_midpoint(seq);
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
  for (int i = 0; i < nsequences; i++) {
    free(sequences[i].numbers);
  }
  free(sequences);
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
