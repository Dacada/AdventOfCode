#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static int cmp(const void *a, const void *b) {
  const unsigned *n1 = a;
  const unsigned *n2 = b;
  return (int)*n1 - (int)*n2;
}

static unsigned parse_number(const char **input) {
  unsigned num = 0;
  char c;
  while (isdigit(c = **input)) {
    num = num * 10 + c - '0';
    *input += 1;
  }
  return num;
}

static unsigned *parse(const char *input, size_t *len) {
  unsigned *numbers = malloc(sizeof(*numbers) * 128);
  size_t capacity = 128;
  numbers[0] = 0;
  size_t length = 1;

  for (; *input != '\0'; input++) {
    if (length >= capacity) {
      capacity *= 2;
    }
    numbers = realloc(numbers, sizeof(*numbers) * capacity);

    unsigned number = parse_number(&input);
    numbers[length] = number;
    length++;
  }

  qsort(numbers, length, sizeof(*numbers), cmp);
  *len = length;
  return numbers;
}

static void solution1(const char *const input, char *const output) {
  size_t numbers_len;
  unsigned *numbers = parse(input, &numbers_len);

  unsigned diff1 = 0;
  unsigned diff3 = 0;
  unsigned current = numbers[0];
  for (size_t i = 1; i < numbers_len; i++) {
    int diff = (int)numbers[i] - (int)current;
    current = numbers[i];

    if (diff == 1) {
      diff1++;
    } else if (diff == 2) {
    } else if (diff == 3) {
      diff3++;
    } else {
      FAIL("unexpected difference: %d", diff);
    }
  }
  diff3++;

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", diff1 * diff3);
  free(numbers);
}

static int get_possible_next_idxs(size_t number_idx, size_t *result, unsigned *numbers, size_t numbers_len) {
  ASSERT(number_idx < numbers_len, "unexpected idx");

  size_t candidate1 = number_idx + 1;
  size_t candidate2 = number_idx + 2;
  size_t candidate3 = number_idx + 3;

  if (candidate1 >= numbers_len || numbers[candidate1] - numbers[number_idx] > 3) {
    return 0;
  } else {
    result[0] = candidate1;
    if (candidate2 >= numbers_len || numbers[candidate2] - numbers[number_idx] > 3) {
      return 1;
    } else {
      result[1] = candidate2;
      if (candidate3 >= numbers_len || numbers[candidate3] - numbers[number_idx] > 3) {
        return 2;
      } else {
        result[2] = candidate3;
        return 3;
      }
    }
  }
}

static unsigned long get_arrangements(size_t number_idx, unsigned *numbers, unsigned long *cache, size_t numbers_len) {
  ASSERT(number_idx < numbers_len, "unexpected idx");

  if (cache[number_idx] > 0) {
    return cache[number_idx];
  }
  DBG("arrangements of %u", numbers[number_idx]);

  size_t possible_next_idxs[3];
  int count_possible_next_idxs = get_possible_next_idxs(number_idx, possible_next_idxs, numbers, numbers_len);
  ASSERT(count_possible_next_idxs >= 0 && count_possible_next_idxs <= 3, "unexpected child count");

  unsigned long result;
  if (count_possible_next_idxs == 0) {
    result = 1;
  } else {
    result = 0;
    for (int i = count_possible_next_idxs - 1; i >= 0; i--) {
      size_t next_idx = possible_next_idxs[i];
      result += get_arrangements(next_idx, numbers, cache, numbers_len);
    }
  }

  cache[number_idx] = result;
  return result;
}

static void solution2(const char *const input, char *const output) {
  size_t numbers_len;
  unsigned *numbers = parse(input, &numbers_len);

  unsigned long *arrangements = malloc(sizeof(*arrangements) * numbers_len);
  memset(arrangements, 0, sizeof(*arrangements) * numbers_len);
  unsigned long result = get_arrangements(0, numbers, arrangements, numbers_len);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
  free(numbers);
  free(arrangements);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
