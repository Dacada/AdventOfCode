#include <aoclib.h>
#include <jsmn/jsmn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_int(const char *const input, const int start, const int end) {
  int number = 0;
  bool negative = false;

  const char c = input[start];
  if (c == '-') {
    negative = true;
  } else {
    number = c - 0x30;
  }

  for (int i = start + 1; i < end; i++) {
    number = number * 10 + input[i] - 0x30;
  }

  if (negative)
    return -number;
  else
    return number;
}

static jsmntok_t *get_tokens(const char *const input, unsigned int *const num_tokens) {
  jsmn_parser parser;
  jsmn_init(&parser);

  const size_t len = strlen(input);
  const unsigned int ntokens = jsmn_parse(&parser, input, len, NULL, 0);
  jsmntok_t *tokens = malloc(ntokens * sizeof(jsmntok_t));

  if (tokens == NULL) {
    perror("malloc");
    abort();
  }

  jsmn_init(&parser);
  jsmn_parse(&parser, input, len, tokens, ntokens);

  *num_tokens = ntokens;
  return tokens;
}

static void solution1(const char *const input, char *const output) {
  unsigned int num_tokens;
  jsmntok_t *tokens = get_tokens(input, &num_tokens);

  int total = 0;
  for (unsigned int i = 0; i < num_tokens; i++) {
    jsmntok_t token = tokens[i];
    if (token.type == JSMN_PRIMITIVE) {
      char c = input[token.start];
      if (c == '-' || (c >= '0' && c <= '9')) {
        int n = parse_int(input, token.start, token.end);
        total += n;
      }
    }
  }

  free(tokens);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

static int sum_without_reds(const char *const input, const jsmntok_t *const tokens, const size_t count,
                            int *const total) {
  char c;
  int j, tmp_total;
  bool found_red;

  if (count == 0) {
    return 0;
  }

  const jsmntok_t token = tokens[0];
  switch (token.type) {
  case JSMN_PRIMITIVE:
    c = input[token.start];
    if (c == '-' || (c >= '0' && c <= '9')) {
      int n = parse_int(input, token.start, token.end);
      *total += n;
    }
    return 1;

  case JSMN_STRING:
    return 1;

  case JSMN_ARRAY:
    j = 0;
    for (int i = 0; i < token.size; i++) {
      j += sum_without_reds(input, tokens + 1 + j, count - j, total);
    }
    return j + 1;

  case JSMN_OBJECT:
    j = 0;
    tmp_total = 0;
    found_red = false;

    for (int i = 0; i < token.size; i++) {
      j += sum_without_reds(input, tokens + 1 + j, count - j, &tmp_total);

      const jsmntok_t *value_token = tokens + 1 + j;
      if (value_token->type == JSMN_STRING &&
          strncmp("red", input + value_token->start, value_token->end - value_token->start) == 0) {
        found_red = true;
        j += 1;
      } else {
        j += sum_without_reds(input, value_token, count - j, &tmp_total);
      }
    }

    if (!found_red) {
      *total += tmp_total;
    }

    return j + 1;
  default:
    FAIL("undefined token");
    return 0;
  }
}

static void solution2(const char *const input, char *const output) {
  unsigned int num_tokens;
  jsmntok_t *tokens = get_tokens(input, &num_tokens);

  int total = 0;
  sum_without_reds(input, tokens, num_tokens, &total);

  free(tokens);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
