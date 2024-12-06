#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

// #define PRINT_LINES

struct line {
  char *pattern;
  int *numbers;
  int pattern_len;
  int numbers_len;
};

static void print_line(const struct line *line, long n) {
#ifdef PRINT_LINES
  if (line->pattern_len <= 0) {
    fputs("'' [", stderr);
  } else {
    fprintf(stderr, "'%s' [", line->pattern);
  }
  for (int i = 0; i < line->numbers_len; i++) {
    fprintf(stderr, "%d", line->numbers[i]);
    if (i < line->numbers_len - 1) {
      fputc(',', stderr);
    }
  }
  fprintf(stderr, "] -> %ld\n", n);
#else
  (void)line;
  (void)n;
#endif
}

static void line_free(struct line *line) {
  free(line->numbers);
  free(line->pattern);
}

static int line_cmp(const struct line *l1, const struct line *l2) {
  int cmp;
  if (l1->pattern_len == 0 && l2->pattern_len == 0) {
    cmp = 0;
  } else if (l1->pattern_len == 0) {
    cmp = strcmp("", l2->pattern);
  } else if (l2->pattern_len == 0) {
    cmp = strcmp(l1->pattern, "");
  } else {
    cmp = strcmp(l1->pattern, l2->pattern);
  }
  if (cmp != 0) {
    return cmp;
  }

  int i;
  for (i = 0; i < l1->numbers_len && i < l2->numbers_len; i++) {
    cmp = l1->numbers[i] - l2->numbers[i];
    if (cmp != 0) {
      return cmp;
    }
  }
  if (l1->numbers_len == l2->numbers_len) {
    return 0;
  } else if (i >= l1->numbers_len) {
    return -1;
  } else {
    return 1;
  }
}

static void line_deepcopy(struct line *copy, const struct line *line) {
  copy->pattern_len = line->pattern_len;
  if (copy->pattern_len <= 0) {
    copy->pattern = NULL;
  } else {
    copy->pattern = strdup(line->pattern);
  }

  size_t s = sizeof(*copy->numbers) * line->numbers_len;
  copy->numbers = malloc(s);
  memcpy(copy->numbers, line->numbers, s);
  copy->numbers_len = line->numbers_len;
}

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error '%c' (%d)", **input, **input);
  int res = 0;
  while (isdigit(**input)) {
    res *= 10;
    res += **input - '0';
    *input += 1;
  }
  return res;
}

static void parse_line(const char **input, struct line *line) {
  const char *tmp = *input;

  line->pattern_len = 0;
  while (!isblank(**input)) {
    line->pattern_len++;
    *input += 1;
  }
  *input += 1;

  line->pattern = strndup(tmp, line->pattern_len);

  int len = 0;
  int cap = 4;
  int *list = malloc(sizeof(*list) * cap);
  while (**input != '\n' && **input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    list[len++] = parse_int(input);
    if (**input == ',') {
      *input += 1;
    }
  }
  if (**input == '\n') {
    *input += 1;
  }

  line->numbers = list;
  line->numbers_len = len;
}

static struct line *parse_input(const char *input, int *length) {
  int len = 0;
  int cap = 4;
  struct line *list = malloc(sizeof(*list) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    parse_line(&input, list + len);
    len++;
  }

  *length = len;
  return list;
}

struct bst {
  bool init;
  struct line key;
  long value;

  struct bst *left;
  struct bst *right;
};

static void bst_init(struct bst *bst) { bst->init = false; }

static bool bst_lookup(struct bst *bst, const struct line *key, long *value) {
  if (bst == NULL || !bst->init) {
    return false;
  }

  int cmp = line_cmp(key, &bst->key);

  struct bst *next = NULL;
  if (cmp < 0) {
    next = bst->left;
  } else if (cmp > 0) {
    next = bst->right;
  } else {
    *value = bst->value;
    return true;
  }

  return bst_lookup(next, key, value);
}

static void bst_add(struct bst *bst, const struct line *key, long value) {
  if (!bst->init) {
    line_deepcopy(&bst->key, key);
    bst->value = value;
    bst->left = NULL;
    bst->right = NULL;
    bst->init = true;
    return;
  }

  int cmp = line_cmp(key, &bst->key);

  struct bst **next = NULL;
  if (cmp < 0) {
    next = &bst->left;
  } else if (cmp > 0) {
    next = &bst->right;
  } else {
    FAIL("adding a value already in the cache!!!");
  }

  if (*next == NULL) {
    *next = malloc(sizeof(*bst->left));
    (*next)->init = false;
  }
  bst_add(*next, key, value);
}

static void bst_free(struct bst *bst) {
  if (bst == NULL || !bst->init) {
    return;
  }

  line_free(&bst->key);

  bst_free(bst->left);
  free(bst->left);

  bst_free(bst->right);
  free(bst->right);
}

static long arrangements(const struct line *line, struct bst *cache) {
  {
    long res = -1;
    if (bst_lookup(cache, line, &res)) {
      return res;
    }
  }

  if (line->numbers_len == 0) {
    bool any = false;
    for (int i = 0; i < line->pattern_len; i++) {
      if (line->pattern[i] == '#') {
        any = true;
        break;
      }
    }
    if (any) {
      print_line(line, 0);
      bst_add(cache, line, 0);
      return 0;
    }
    print_line(line, 1);
    bst_add(cache, line, 1);
    return 1;
  }
  if (line->pattern_len == 0) {
    print_line(line, 0);
    bst_add(cache, line, 0);
    return 0;
  }

  char c = line->pattern[0];
  int n = line->numbers[0];

  long rest;
  if (c == '.' || c == '?') {
    struct line new_line;
    new_line.pattern = line->pattern + 1;
    new_line.pattern_len = line->pattern_len - 1;
    new_line.numbers = line->numbers;
    new_line.numbers_len = line->numbers_len;
    rest = arrangements(&new_line, cache);
  } else {
    rest = 0;
  }

  if (c == '.') {
    print_line(line, rest);
    bst_add(cache, line, rest);
    return rest;
  }

  int spaces = 0;
  for (int i = 0; i < line->pattern_len; i++) {
    char cc = line->pattern[i];
    if (cc == '#' || cc == '?') {
      spaces += 1;
    } else {
      break;
    }
  }

  if (spaces < n) {
    print_line(line, rest);
    bst_add(cache, line, rest);
    return rest;
  }

  if (n < line->pattern_len && line->pattern[n] == '#') {
    print_line(line, rest);
    bst_add(cache, line, rest);
    return rest;
  }

  struct line new_line;
  new_line.pattern = line->pattern + n + 1;
  new_line.pattern_len = line->pattern_len - n - 1;
  if (new_line.pattern_len < 0) {
    new_line.pattern_len = 0;
  }
  new_line.numbers = line->numbers + 1;
  new_line.numbers_len = line->numbers_len - 1;

  long r = rest + arrangements(&new_line, cache);
  print_line(line, r);
  bst_add(cache, line, r);
  return r;
}

static void expand_line(struct line *line) {
  line->pattern = realloc(line->pattern, sizeof(*line->pattern) * line->pattern_len * 5 + 4 + 1);
  line->numbers = realloc(line->numbers, sizeof(*line->numbers) * line->numbers_len * 5);

  for (int i = 1; i < 5; i++) {
    memcpy(line->pattern + line->pattern_len * i + i, line->pattern, line->pattern_len * sizeof(*line->pattern));
    memcpy(line->numbers + line->numbers_len * i, line->numbers, line->numbers_len * sizeof(*line->numbers));
    line->pattern[line->pattern_len * i + i - 1] = '?';
  }

  line->pattern_len *= 5;
  line->pattern_len += 4;
  line->numbers_len *= 5;

  line->pattern[line->pattern_len] = '\0';
}

static void solution(const char *const input, char *const output, bool expand) {
  int len;
  struct line *lines = parse_input(input, &len);
  struct bst cache;
  bst_init(&cache);

  long res = 0;
  for (int i = 0; i < len; i++) {
    struct line *line = lines + i;
    if (expand) {
      expand_line(line);
    }
    res += arrangements(line, &cache);
    line_free(line);
  }

  bst_free(&cache);
  free(lines);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
}

static void solution1(const char *const input, char *const output) { solution(input, output, false); }

static void solution2(const char *const input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
