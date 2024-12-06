#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static char *parse_input(const char *const input, size_t *const height, size_t *const width) {
  for (*width = 0;; *width += 1) {
    if (input[*width] != '0' && input[*width] != '1') {
      break;
    }
  }

  size_t size = 128;
  char *numbers = malloc(size);
  size_t j = 0;
  for (size_t i = 0;; i++) {
    if (input[i] == '\0') {
      break;
    }
    if (j >= size) {
      size *= 2;
      numbers = realloc(numbers, size);
    }
    if (input[i] == '1' || input[i] == '0') {
      numbers[j] = input[i];
      j++;
    }
  }

  *height = j / *width;
  return numbers;
}

static void solution1(const char *const input, char *const output) {
  size_t height, width;
  char *numbers = parse_input(input, &height, &width);

  unsigned gamma = 0;
  unsigned epsilon = 0;

  for (size_t i = 0; i < width; i++) {
    unsigned ones = 0;
    for (size_t j = 0; j < height; j++) {
      if (numbers[j * width + i] == '1') {
        ones++;
      }
    }

    gamma *= 2;
    epsilon *= 2;
    if (ones >= height / 2 + 1) {
      gamma += 1;
    } else {
      epsilon += 1;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", gamma * epsilon);
  free(numbers);
}

static void solution2(const char *const input, char *const output) {
  size_t height, width;
  char *numbers = parse_input(input, &height, &width);

  char *oxygen_criteria = malloc(width * sizeof(*oxygen_criteria));
  char *co2_criteria = malloc(width * sizeof(*co2_criteria));

  bool *oxygen_discarded = malloc(height * sizeof(*oxygen_discarded));
  bool *co2_discarded = malloc(height * sizeof(*co2_discarded));

  memset(oxygen_discarded, 0, height * sizeof(*oxygen_discarded));
  memset(co2_discarded, 0, height * sizeof(*co2_discarded));

  bool oxygen_found = false;
  bool co2_found = false;

  unsigned oxygen = 0;
  unsigned co2 = 0;

  for (size_t i = 0; i < width; i++) {
    DBG("%lu", i);
    if (oxygen_found && co2_found) {
      break;
    }

    unsigned oxygen_ones_count = 0;
    unsigned oxygen_zeros_count = 0;
    unsigned co2_ones_count = 0;
    unsigned co2_zeros_count = 0;
    size_t oxygen_last_one = 0;
    size_t oxygen_last_zero = 0;
    size_t co2_last_one = 0;
    size_t co2_last_zero = 0;
    for (size_t j = 0; j < height; j++) {
      bool oxygen_valid = !oxygen_found && !oxygen_discarded[j];
      bool co2_valid = !co2_found && !co2_discarded[j];
      if (oxygen_valid) {
        oxygen_valid = strncmp(&numbers[j * width], oxygen_criteria, i) == 0;
        oxygen_discarded[j] = !oxygen_valid;
      }
      if (co2_valid) {
        co2_valid = strncmp(&numbers[j * width], co2_criteria, i) == 0;
        co2_discarded[j] = !co2_valid;
      }

      char c = numbers[j * width + i];
      if (c == '0') {
        if (oxygen_valid) {
          oxygen_zeros_count++;
          oxygen_last_zero = j;
        }
        if (co2_valid) {
          co2_zeros_count++;
          co2_last_zero = j;
        }
      } else if (c == '1') {
        if (oxygen_valid) {
          oxygen_ones_count++;
          oxygen_last_one = j;
        }
        if (co2_valid) {
          co2_ones_count++;
          co2_last_one = j;
        }
      } else {
        ASSERT(false, "invalid character '%c'", c);
      }
    }

    unsigned oxygen_count = oxygen_ones_count + oxygen_zeros_count;
    unsigned co2_count = co2_ones_count + co2_zeros_count;
    ASSERT(oxygen_found || oxygen_count > 0, "all oxygen candidates discarded");
    ASSERT(co2_found || co2_count > 0, "all co2 candidates discarded");
    DBG("oxygen_count = %u", oxygen_count);
    DBG("co2_count = %u", co2_count);

    if (!oxygen_found) {
      if (oxygen_count == 1) {
        size_t j;
        if (oxygen_ones_count == 1) {
          j = oxygen_last_one;
        } else {
          j = oxygen_last_zero;
        }
        oxygen = 0;
        for (size_t k = 0; k < width; k++) {
          oxygen *= 2;
          oxygen += numbers[j * width + k] - '0';
        }
        oxygen_found = true;
      } else if (oxygen_ones_count >= oxygen_zeros_count) {
        oxygen_criteria[i] = '1';
      } else {
        oxygen_criteria[i] = '0';
      }
    }

    if (!co2_found) {
      if (co2_count == 1) {
        size_t j;
        if (co2_ones_count == 1) {
          j = co2_last_one;
        } else {
          j = co2_last_zero;
        }
        co2 = 0;
        for (size_t k = 0; k < width; k++) {
          co2 *= 2;
          co2 += numbers[j * width + k] - '0';
        }
        co2_found = true;
      } else if (co2_zeros_count <= co2_ones_count) {
        co2_criteria[i] = '0';
      } else {
        co2_criteria[i] = '1';
      }
    }
  }

  if (!oxygen_found) {
    oxygen = 0;
    for (size_t i = 0; i < width; i++) {
      oxygen *= 2;
      oxygen += oxygen_criteria[i] - '0';
    }
  }
  if (!co2_found) {
    co2 = 0;
    for (size_t i = 0; i < width; i++) {
      co2 *= 2;
      co2 += co2_criteria[i] - '0';
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", oxygen * co2);
  free(numbers);
  free(oxygen_criteria);
  free(co2_criteria);
  free(oxygen_discarded);
  free(co2_discarded);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
