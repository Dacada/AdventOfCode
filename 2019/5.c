#include <aoclib.h>
#include <stdbool.h>
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

  bool neg = false;
  for (size_t i = 0; i < 1 << 16; i++) {
    char c = input[i];

    if (c == ',') {
      if (neg) {
        program[program_i] = -program[program_i];
        neg = false;
      }

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
    } else if (c == '-') {
      neg = true; // !!!! a - in any position in a number will make it negative
    } else {
      break;
    }
  }

  *size = program_i + 1;

  return program;
}

__attribute__((const)) static int get_mode(int modes, int i) {
  while (i > 1) {
    modes /= 10;
    i--;
  }
  return modes % 10;
}

static int get_argument(int *const program, int i, int mode) {
  int arg = program[i];
  switch (mode) {
  case 0:
    return program[arg];
  case 1:
    return arg;
  default:
    FAIL("Invalid mode: %d", mode);
  }
}

static int getarg(int *const program, int offset, int modes, int i) {
  return get_argument(program, offset + i, get_mode(modes, i));
}

static void prog_sum(int *const program, int start, int modes) {
  int arg1 = getarg(program, start, modes, 1);
  int arg2 = getarg(program, start, modes, 2);
  int res_index = program[start + 3];
  program[res_index] = arg1 + arg2;
  DBG("[%d] = %d + %d", res_index, arg1, arg2);
}

static void prog_mult(int *const program, int start, int modes) {
  int arg1 = getarg(program, start, modes, 1);
  int arg2 = getarg(program, start, modes, 2);
  int res_index = program[start + 3];
  program[res_index] = arg1 * arg2;
  DBG("[%d] = %d * %d", res_index, arg1, arg2);
}

static void prog_input(int *const program, int start, int modes, int input) {
  (void)modes;
  int res_index = program[start + 1];
  program[res_index] = input;
  DBG("[%d] = input()", res_index);
}

static int prog_output(int *const program, int start, int modes) {
  int output = getarg(program, start, modes, 1);
  DBG("output %d", output);
  return output;
}

static int prog_jumpif(int *const program, int start, int modes, bool what) {
  int arg1 = getarg(program, start, modes, 1);
  bool cond = arg1 != 0;

  if ((cond && what) || (!cond && !what)) {
    return getarg(program, start, modes, 2);
  } else {
    return start + 3;
  }
}

static void prog_lessthan(int *const program, int start, int modes) {
  int arg1 = getarg(program, start, modes, 1);
  int arg2 = getarg(program, start, modes, 2);
  int res_index = program[start + 3];
  if (arg1 < arg2) {
    program[res_index] = 1;
  } else {
    program[res_index] = 0;
  }
}

static void prog_equals(int *const program, int start, int modes) {
  int arg1 = getarg(program, start, modes, 1);
  int arg2 = getarg(program, start, modes, 2);
  int res_index = program[start + 3];
  if (arg1 == arg2) {
    program[res_index] = 1;
  } else {
    program[res_index] = 0;
  }
}

static void run_program(int *const program, size_t program_size, int(get_input(void *)), void *input_arg,
                        void(give_output(int, void *)), void *output_arg) {
  size_t pc = 0;
  while (pc < program_size) {
    int opcode = program[pc] % 100;
    int modes = program[pc] / 100;

    switch (opcode) {
    case 1:
      prog_sum(program, pc, modes);
      pc += 4;
      break;
    case 2:
      prog_mult(program, pc, modes);
      pc += 4;
      break;
    case 3:
      prog_input(program, pc, modes, get_input(input_arg));
      pc += 2;
      break;
    case 4:
      give_output(prog_output(program, pc, modes), output_arg);
      pc += 2;
      break;
    case 5:
      pc = prog_jumpif(program, pc, modes, true);
      break;
    case 6:
      pc = prog_jumpif(program, pc, modes, false);
      break;
    case 7:
      prog_lessthan(program, pc, modes);
      pc += 4;
      break;
    case 8:
      prog_equals(program, pc, modes);
      pc += 4;
      break;
    case 99:
      return;
    default:
      FAIL("Found an unexpected opcode: %d (modes: %d)", opcode, modes);
    }
  }
}

static int input_getter(void *num) { return *(int *)num; }

static void output_setter(int output, void *num) { *(int *)num = output; }

static void solution(const char *const input, char *const output, int program_input) {
  size_t program_size;
  int *program = parse_program(input, &program_size);
  if (program == NULL) {
    snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
    return;
  }

  int program_output = -1;
  run_program(program, program_size, input_getter, &program_input, output_setter, &program_output);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", program_output);
  free(program);
}

static void solution1(const char *const input, char *const output) { solution(input, output, 1); }

static void solution2(const char *const input, char *const output) { solution(input, output, 5); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
