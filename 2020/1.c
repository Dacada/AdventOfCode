#include <aoclib.h>
#include <stdio.h>

static int parse_int(const char *const text, size_t *const idx) {
  int n = 0;
  int i;
  for (i = 0;; i++) {
    char c = text[*idx + i];
    if (c < '0' || c > '9') {
      break;
    }
    n = n * 10 + (c - '0');
  }
  *idx += i;
  return n;
}

static int *get_numbers(const char *const input, size_t *const number_count) {
  size_t count = 0;
  for (size_t i = 0;; i++) {
    if (input[i] == '\0') {
      count++;
      break;
    } else if (input[i] == '\n') {
      count++;
    }
  }

  int *numbers = malloc(sizeof(int) * count);
  ASSERT(numbers != NULL, "memory error");
  size_t j = 0;
  for (size_t i = 0;; i++) {
    if (input[i] == '\0') {
      break;
    }
    ASSERT(j < count, "too many numbers");
    numbers[j++] = parse_int(input, &i);
  }

  *number_count = count;
  return numbers;
}

static void solution1(const char *const input, char *const output) {
  size_t number_count;
  int *numbers = get_numbers(input, &number_count);

  int result = -1;
  for (size_t i = 0; i < number_count; i++) {
    for (size_t j = 0; j < number_count; j++) {
      if (numbers[i] + numbers[j] == 2020) {
        result = numbers[i] * numbers[j];
        goto end;
      }
    }
  }

end:
  free(numbers);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *const input, char *const output) {
  size_t number_count;
  int *numbers = get_numbers(input, &number_count);

  int result = -1;
  for (size_t i = 0; i < number_count; i++) {
    for (size_t j = 0; j < number_count; j++) {
      for (size_t k = 0; k < number_count; k++) {
        long sum = (long)numbers[i] + (long)numbers[j] + (long)numbers[k];
        if (sum == 2020) {
          result = numbers[i] * numbers[j] * numbers[k];
          goto end;
        }
      }
    }
  }

end:
  free(numbers);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
