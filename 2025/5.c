#include <aoclib.h>
#include <stdio.h>
#include <stdlib.h>

struct range {
  long start, end;
};

#define inrange(n, range) ((range).start <= (n) && (n) <= (range).end)

static int range_cmp(const void *vr1, const void *vr2) {
  const struct range *r1 = vr1;
  const struct range *r2 = vr2;

  return (r1->start > r2->start) - (r1->start < r2->start);
}

static bool parse_range(const char **input, void *element) {
  if (**input == '\n') {
    return false;
  }

  struct range *range = element;

  aoc_skip_space(input);
  range->start = aoc_parse_long(input);
  aoc_expect_char(input, '-');
  range->end = aoc_parse_long(input);
  aoc_expect_char(input, '\n');
  return true;
}

static bool parse_ingredient(const char **input, void *element) {
  if (**input == '\0') {
    return false;
  }

  long *ingredient = element;

  aoc_skip_space(input);
  *ingredient = aoc_parse_long(input);
  if (**input != '\0') {
    aoc_expect_char(input, '\n');
  }
  aoc_skip_space(input);
  return true;
}

static void solution1(const char *input, char *const output) {
  int nranges;
  struct range *ranges = aoc_parse_sequence(&input, &nranges, sizeof(*ranges), 16, parse_range);

  int ningredients;
  long *ingredients = aoc_parse_sequence(&input, &ningredients, sizeof(*ingredients), 16, parse_ingredient);

  int count = 0;
  for (int i = 0; i < ningredients; i++) {
    long ingredient = ingredients[i];
    bool spoiled = true;
    for (int j = 0; j < nranges; j++) {
      struct range range = ranges[j];
      if (inrange(ingredient, range)) {
        spoiled = false;
        break;
      }
    }
    if (!spoiled) {
      count += 1;
    }
  }

  free(ranges);
  free(ingredients);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static struct range *merge_sorted_ranges(const struct range *ranges, int nranges, int *nmerged_ptr) {
  struct range *merged = malloc(sizeof(*merged) * nranges);
  int nmerged = 0;

  struct range current = ranges[0];

  for (int i = 1; i < nranges; i++) {
    struct range candidate = ranges[i];
    if (candidate.start <= current.end) {
      if (candidate.end > current.end) {
        current.end = candidate.end;
      }
    } else {
      merged[nmerged++] = current;
      current = candidate;
    }
  }

  merged[nmerged++] = current;
  *nmerged_ptr = nmerged;
  return merged;
}

static void solution2(const char *input, char *const output) {
  int nranges;
  struct range *ranges = aoc_parse_sequence(&input, &nranges, sizeof(*ranges), 16, parse_range);
  qsort(ranges, nranges, sizeof(*ranges), range_cmp);

  int nmerged;
  struct range *merged = merge_sorted_ranges(ranges, nranges, &nmerged);
  free(ranges);

  long total = 0;
  for (int i = 0; i < nmerged; i++) {
    struct range range = merged[i];
    total += range.end - range.start + 1;
  }

  free(merged);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
