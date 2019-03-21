#include <aoclib.h>
#include <jsmn/jsmn.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static int parse_int(char *input, int start, int end) {
  int number = 0;
  bool negative = false;

  char c = input[start];
  if (c == '-') {
    negative = true;
  } else {
    number = c - 0x30;
  }

  for (int i=start+1; i<end; i++) {
    number = number * 10 + input[i] - 0x30;
  }

  if (negative)
    return -number;
  else
    return number;
}

static jsmntok_t *get_tokens(char *input, unsigned int *num_tokens) {
  jsmn_parser parser;
  jsmn_init(&parser);

  size_t len = strlen(input);
  unsigned int ntokens = jsmn_parse(&parser, input, len, NULL, 0);
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

static void solution1(char *input, char *output) {
  unsigned int num_tokens;
  jsmntok_t *tokens = get_tokens(input, &num_tokens);

  int total = 0;
  for (unsigned int i=0; i<num_tokens; i++) {
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

static int sum_without_reds(char *input, jsmntok_t *tokens) {
}

static void solution2(char *input, char *output) {
  unsigned int num_tokens;
  jsmntok_t *tokens = get_tokens(input, &num_tokens);

  int total = sum_without_reds(input, tokens);

  free(tokens);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
