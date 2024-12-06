#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define NLETTERS ('z' - 'a' + 1)

struct letter_count {
  char letter;
  unsigned count;
};

static int letters_cmp(const void *va, const void *vb) {
  const struct letter_count *a = va, *b = vb;
  int count_diff = a->count - b->count;
  if (count_diff == 0) {
    return b->letter - a->letter;
  } else {
    return count_diff;
  }
}

static void compute_hash(struct letter_count *const letters, size_t letters_len, char *const buff, size_t buff_len) {
  qsort(letters, letters_len, sizeof(*letters), letters_cmp);
  for (size_t i = 0; i < buff_len; i++) {
    buff[i] = letters[letters_len - 1 - i].letter;
  }
}

static void solution(const char *input, char *const output, bool count) {
  unsigned result = 0;

  while (*input) {
    struct letter_count letters[NLETTERS];
    for (char c = 'a'; c <= 'z'; c++) {
      letters[c - 'a'].letter = c;
      letters[c - 'a'].count = 0;
    }

    char name[64] = {0};
    int j = 0;
    while (!isdigit(*input)) {
      if (*input != '-') {
        ASSERT(islower(*input), "Unexpected character");
        letters[*input - 'a'].count++;
      }
      if (!count) {
        if (j < 64) {
          name[j++] = *input;
        }
      }
      input++;
    }

    unsigned id = 0;
    while (isdigit(*input)) {
      id = id * 10 + *input - '0';
      input++;
    }

    ASSERT(*input == '[', "Unexpected character");
    input++;

    char given_hash[5];
    int i = 0;
    while (*input != ']') {
      given_hash[i++] = *(input++);
    }

    input++;
    ASSERT(*input == '\n', "Unexpected character");
    input++;

    char computed_hash[5];
    compute_hash(letters, NLETTERS, computed_hash, 5);

    if (strncmp(given_hash, computed_hash, 5) == 0) {
      if (count) {
        result += id;
      } else {
        for (int k = 0; k < 64; k++) {
          if (name[k] == 0) {
            break;
          } else if (name[k] == '-') {
            name[k] = ' ';
          } else {
            name[k] = (((name[k] - 'a') + id) % NLETTERS) + 'a';
          }
        }
        if (strncmp("north", name, 5) == 0) {
          result = id;
          break;
        }
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

static void solution1(const char *const input, char *const output) { solution(input, output, true); }

static void solution2(const char *const input, char *const output) { solution(input, output, false); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
