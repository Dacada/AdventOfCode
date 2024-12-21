#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>

#define TRIELEN 26

struct slice {
  const char *str;
  size_t len;
};

static struct slice slice_adv(struct slice s, size_t amnt) {
  ASSERT(s.len >= amnt, "advance past end of slice (len=%lu amnt=%lu)", s.len, amnt);
  s.str += amnt;
  s.len -= amnt;
  return s;
}

struct trie {
  struct trie *children[TRIELEN];
  long value;
};

static void trie_init(struct trie *trie) {
  for (int i = 0; i < TRIELEN; i++) {
    trie->children[i] = NULL;
  }
  trie->value = -1;
}

static void trie_free(struct trie *trie) {
  if (trie == NULL) {
    return;
  }
  for (int i = 0; i < TRIELEN; i++) {
    trie_free(trie->children[i]);
    free(trie->children[i]);
  }
}

static void trie_add(struct trie *trie, const struct slice str, long value) {
  if (str.len == 0) {
    ASSERT(trie->value == -1, "overwrite attempted");
    trie->value = value;
    return;
  }

  ASSERT(*str.str >= 'a' && *str.str <= 'z', "invalid string");
  struct trie **next = &trie->children[*str.str - 'a'];
  if (*next == NULL) {
    *next = aoc_malloc(sizeof(**next));
    trie_init(*next);
  }

  struct slice new = slice_adv(str, 1);
  trie_add(*next, new, value);
}

__attribute__((pure)) static long trie_lookup(struct trie *trie, const struct slice str) {
  if (str.len == 0) {
    return trie->value;
  }

  ASSERT(*str.str >= 'a' && *str.str <= 'z', "invalid string");
  struct trie *next = trie->children[*str.str - 'a'];
  if (next == NULL) {
    return -1;
  }

  struct slice new = slice_adv(str, 1);
  return trie_lookup(next, new);
}

static struct slice parse_lowercase_sequence(const char **input) {
  struct slice slice;
  slice.str = *input;
  slice.len = 0;

  while (islower(**input)) {
    *input += 1;
    slice.len++;
  }

  return slice;
}

static void parse_fragments(const char **input, struct trie *fragments) {
  for (;;) {
    struct slice fragment = parse_lowercase_sequence(input);
    trie_add(fragments, fragment, 1);
    if (**input != ',') {
      break;
    }
    *input += 1;
    aoc_skip_space(input);
  }
}

static struct slice *parse_patterns(const char **input, int *len) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(struct slice), 8);

  for (;;) {
    struct slice *slice = aoc_dynarr_grow(&arr, 1);
    *slice = parse_lowercase_sequence(input);
    aoc_skip_space(input);
    if (**input == '\0') {
      break;
    }
  }

  *len = arr.len;
  return arr.data;
}

static struct slice *parse_input(const char *input, struct trie *fragments, int *npatterns) {
  parse_fragments(&input, fragments);
  aoc_skip_space(&input);
  return parse_patterns(&input, npatterns);
}

int recurse_depth = 0;
static long pattern_possible(struct slice pattern, const struct trie *fragments, struct trie *memory) {
  struct slice orig = pattern;
  recurse_depth++;

  long res = trie_lookup(memory, pattern);
  if (res != -1) {
    DBG("memoization: %.*s = %ld", (int)pattern.len, pattern.str, res);
    return res;
  }

  const struct trie *trie = fragments;
  long count = 0;

  for (;;) {
    trie = trie->children[*pattern.str - 'a'];
    if (trie == NULL) {
      /* DBG("%*c%.*s: trie cell has no children, return count=%ld", recurse_depth, ' ', (int)pattern.len, pattern.str,
       */
      /*     count); */
      recurse_depth--;
      trie_add(memory, orig, count);
      DBG("%.*s: %ld", (int)orig.len, orig.str, count);
      return count;
    }
    /* DBG("%*c%.*s: trie cell has a matching child, will continue", recurse_depth, ' ', (int)pattern.len, pattern.str);
     */

    pattern = slice_adv(pattern, 1);

    if (trie->value != -1) {
      /* DBG("%*c%.*s: trie cell is valid", recurse_depth, ' ', (int)pattern.len, pattern.str); */
      if (pattern.len == 0) {
        /* DBG("%*c%.*s: pattern will be exhausted, we can increase count", recurse_depth, ' ', (int)pattern.len, */
        /*     pattern.str); */
        count += 1;
      } else {
        /* DBG("%*c%.*s: pattern still not exhausted, recursing", recurse_depth, ' ', (int)pattern.len, pattern.str); */
        count += pattern_possible(pattern, fragments, memory);
        /* DBG("%*c%.*s: after recursing, count is %ld", recurse_depth, ' ', (int)pattern.len, pattern.str, count); */
      }
    } else {
      /* DBG("%*c%.*s: trie has no matching child", recurse_depth, ' ', (int)pattern.len, pattern.str); */
    }
    if (pattern.len == 0) {
      /* DBG("%*c%.*s: pattern is exhausted, returning %ld", recurse_depth, ' ', (int)pattern.len, pattern.str, count);
       */
      recurse_depth--;
      trie_add(memory, orig, count);
      DBG("%.*s: %ld", (int)orig.len, orig.str, count);
      return count;
    }
  }
}

static void solution1(const char *input, char *const output) {
  struct trie fragments;
  trie_init(&fragments);
  int npatterns;
  struct slice *patterns = parse_input(input, &fragments, &npatterns);

  int count = 0;
  struct trie memory;
  trie_init(&memory);
  for (int i = 0; i < npatterns; i++) {
    if (pattern_possible(patterns[i], &fragments, &memory)) {
      count++;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
  trie_free(&fragments);
  trie_free(&memory);
  free(patterns);
}

static void solution2(const char *input, char *const output) {
  struct trie fragments;
  trie_init(&fragments);
  int npatterns;
  struct slice *patterns = parse_input(input, &fragments, &npatterns);

  long count = 0;
  struct trie memory;
  trie_init(&memory);
  for (int i = 0; i < npatterns; i++) {
    long n = pattern_possible(patterns[i], &fragments, &memory);
    DBG(" -> %.*s: %ld", (int)patterns[i].len, patterns[i].str, n);
    count += n;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", count);
  trie_free(&fragments);
  trie_free(&memory);
  free(patterns);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
