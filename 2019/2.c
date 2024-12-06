#include <aoclib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int *parse_program(const char *const input, size_t *const size) {
  int *program = calloc(32, sizeof(int));
  if (program == NULL) {
    perror("malloc");
    return NULL;
  }

  size_t program_i = 0;
  size_t program_size = 32;

  for (size_t i = 0; i < 1 << 16; i++) {
    char c = input[i];

    if (c == ',') {
      program_i++;
      while (program_i >= program_size) {
        program = realloc(program, sizeof(int) * program_size * 2);
        memset(program + program_size, 0, program_size * sizeof(int));
        program_size *= 2;
        if (program == NULL) {
          perror("realloc");
          return NULL;
        }
      }
    } else if (c >= '0' && c <= '9') {
      program[program_i] = program[program_i] * 10 + c - '0';
    } else {
      break;
    }
  }

  *size = program_i + 1;

  return program;
}

static void run_program(int *const program, size_t program_size) {
  for (size_t i = 0; i < program_size; i += 4) {
    int op = program[i];
    int arg1 = program[i + 1];
    int arg2 = program[i + 2];
    int res = program[i + 3];

    switch (op) {
    case 99:
      return;
    case 2:
      program[res] = program[arg1] * program[arg2];
      DBG("[%d] = %d * %d", res, program[arg1], program[arg2]);
      break;
    case 1:
      program[res] = program[arg1] + program[arg2];
      DBG("[%d] = %d + %d", res, program[arg1], program[arg2]);
      break;
    }
  }
}

static void solution1(const char *const input, char *const output) {
  size_t program_size;
  int *program = parse_program(input, &program_size);
  if (program == NULL) {
    snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
    return;
  }

  program[1] = 12;
  program[2] = 2;
  run_program(program, program_size);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", program[0]);
  free(program);
}

static void solution2(const char *const input, char *const output) {
  size_t program_size;
  int *original, *program;
  original = parse_program(input, &program_size);
  program = malloc(sizeof(int) * program_size);

  int noun = 0;
  int verb = 0;
  int result = 0;
  for (;;) {
    memcpy(program, original, sizeof(int) * program_size);
    program[1] = noun;
    program[2] = verb;
    run_program(program, program_size);
    result = program[0];

    if (result == 19690720) {
      break;
    } else {
      noun++;
      if (noun == 100) {
        if (verb == 100) {
          break;
        } else {
          noun = 0;
          verb++;
        }
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", noun * 100 + verb);
  free(program);
  free(original);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
