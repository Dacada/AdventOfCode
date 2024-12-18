#define _DEFAULT_SOURCE

#include "intcode.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long *parse_program(const char *const input, size_t *const size) {
  long *program = calloc(32, sizeof(*program));
  if (program == NULL) {
    perror("calloc");
    return NULL;
  }

  size_t program_i = 0;
  size_t program_size = 32;

  bool neg = false;
  for (size_t i = 0; i < 1 << 16; i++) {
    char c = input[i];

    if (isspace(c)) {
      continue;
    } else if (c == ',') {
      if (neg) {
        program[program_i] = -program[program_i];
        neg = false;
      }

      program_i++;
      while (program_i >= program_size) {
        size_t new_program_size = program_size * 2;
        void *new_program = realloc(program, sizeof(*program) * new_program_size);
        if (new_program == NULL) {
          free(program);
          perror("realloc");
          return NULL;
        }
        program = new_program;

        memset(program + program_size, 0, program_size * sizeof(*program));
        program_size = new_program_size;
      }
    } else if (c >= '0' && c <= '9') {
      program[program_i] = program[program_i] * 10 + c - '0';
    } else if (c == '-') {
      neg = true; // !!!! a - in any position in a number will make it negative
    } else {
      break;
    }
  }

  *size = program_size;

  return program;
}

void machine_init(struct IntCodeMachine *const machine, const char *const input) {
  machine->program = parse_program(input, &machine->program_size);
  machine->relative_base = 0;

  machine->running = false;
  machine->has_input = false;
  machine->has_output = false;
}

void machine_clone(struct IntCodeMachine *const dest, const struct IntCodeMachine *const src) {
  long *ptr = malloc(src->program_size * sizeof(*dest->program));
  machine_clone_static(dest, src, ptr);
}

void machine_clone_static(struct IntCodeMachine *const dest, const struct IntCodeMachine *const src, long *ptr) {
  dest->program = ptr;
  memcpy(dest->program, src->program, src->program_size * sizeof(*dest->program));

  dest->program_size = src->program_size;
  dest->relative_base = src->relative_base;
  dest->running = src->running;
  dest->pc = src->pc;
  dest->has_input = src->has_input;
  dest->input = src->input;
  dest->has_output = src->has_output;
  dest->output = src->output;
}

void machine_free(struct IntCodeMachine *const machine) { free(machine->program); }

bool machine_recv_output(struct IntCodeMachine *const machine, long *const output) {
#ifdef DEBUG
  if (!machine->running) {
    return false;
  }
#endif

  if (machine->has_output) {
    machine->has_output = false;
    *output = machine->output;
    return true;
  } else {
    return false;
  }
}

bool machine_send_input(struct IntCodeMachine *const machine, long input) {
#ifdef DEBUG
  if (!machine->running) {
    return false;
  }
#endif

  if (machine->has_input) {
    return false;
  } else {
    machine->has_input = true;
    machine->input = input;
    return true;
  }
}

char *machine_recv_output_string(struct IntCodeMachine *const machine) {
#ifdef DEBUG
  if (!machine->running) {
    return NULL;
  }
#endif

  char *str = NULL;
  size_t ri = 0;
  size_t rs = 0;

  while (machine->has_output) {
    while (ri >= rs) {
      rs = rs == 0 ? 16 : rs * 2;
      void *tmp = realloc(str, rs * sizeof *str);
      if (tmp == NULL) {
        free(str);
        perror("realloc");
        return NULL;
      }
      str = tmp;
    }

    long output = machine->output;
    machine->has_output = false;
    machine_run(machine);

    if (output < 0 || output > CHAR_MAX || !isascii(output)) {
      machine->has_output = true; // push back
      break;
    }
    str[ri++] = (char)output;
  }

  if (ri == 0) {
    return NULL;
  }

  void *tmp = realloc(str, (ri + 1) * sizeof *str);
  if (tmp == NULL && ri > 0) {
    free(str);
    perror("realloc");
    return NULL;
  }
  str = tmp;

  str[ri] = '\0';
  return str;
}

void machine_discard_output_string(struct IntCodeMachine *const machine) {
  while (machine->has_output) {
    if (machine->output < 0 || machine->output > CHAR_MAX || !isascii(machine->output)) {
      // do not acknowledge
      break;
    }
    machine->has_output = false;
    machine_run(machine);
  }
}

bool machine_send_input_string(struct IntCodeMachine *const machine, const char *const input) {
#ifdef DEBUG
  if (!machine->running) {
    return false;
  }
#endif

  for (size_t i = 0;; i++) {
    char c = input[i];
    if (c == '\0') {
      return true;
    }

    if (!machine_send_input(machine, (long)c)) {
      return false;
    }
    machine_run(machine);
  }
}

static long get_mode(long modes, size_t i) {
  while (i > 1) {
    modes /= 10;
    i--;
  }
  long m = modes % 10;
  return m;
}

static void ensure_memory_size(struct IntCodeMachine *const machine, size_t index) {
#ifdef DEBUG
  if (index > 1 << 20) {
    fprintf(stderr, "Cowardly refusing to allocate memory to access index %lu\n", index);
    abort();
  }
#endif

  bool modified_size = false;
  size_t size = machine->program_size;

  while (index >= size) {
    size *= 2;
    modified_size = true;
  }

  if (modified_size) {
    machine->program = realloc(machine->program, size * sizeof(*machine->program));
    if (machine->program == NULL) {
      perror("realloc");
      abort();
    }

    memset(machine->program + machine->program_size, 0, (size - machine->program_size) * sizeof(*machine->program));

    machine->program_size = size;
  }
}

static long read_memory(struct IntCodeMachine *const machine, size_t index) {
  ensure_memory_size(machine, index);
  return machine->program[index];
}

static void write_memory(struct IntCodeMachine *const machine, size_t index, long data) {
  ensure_memory_size(machine, index);
  machine->program[index] = data;
}

static long get_argument(struct IntCodeMachine *machine, size_t i, long mode) {
  long arg = read_memory(machine, i);

  switch (mode) {
  case 0:
    return read_memory(machine, arg);
  case 1:
    return arg;
  case 2:
    return read_memory(machine, machine->relative_base + arg);
  default:
#ifdef DEBUG
    fprintf(stderr, "Warning: Attempt to get argument in invalid mode %ld. Defaulting to mode 0.\n", mode);
#endif
    return read_memory(machine, arg);
  }
}

static void set_argument(struct IntCodeMachine *machine, size_t i, long mode, long value) {
  long arg = read_memory(machine, i);

  switch (mode) {
  case 0:
    write_memory(machine, arg, value);
    break;
  case 1:
#ifdef DEBUG
    fprintf(stderr, "Warning: Attempt to set argument with invalid immediate mode. Defaulting to mode 0.\n");
#endif
    write_memory(machine, arg, value);
    break;
  case 2:
    write_memory(machine, machine->relative_base + arg, value);
    break;
  default:
#ifdef DEBUG
    fprintf(stderr, "Warning: Attempt to get argument in invalid mode %ld. Defaulting to mode 0.\n", mode);
#endif
    write_memory(machine, arg, value);
  }
}

static long getarg(struct IntCodeMachine *machine, size_t offset, long modes, size_t i) {
  return get_argument(machine, offset + i, get_mode(modes, i));
}

static void setarg(struct IntCodeMachine *machine, size_t offset, long modes, size_t i, long value) {
  set_argument(machine, offset + i, get_mode(modes, i), value);
}

static void prog_sum(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg1 = getarg(machine, start, modes, 1);
  long arg2 = getarg(machine, start, modes, 2);
  setarg(machine, start, modes, 3, arg1 + arg2);
}

static void prog_mult(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg1 = getarg(machine, start, modes, 1);
  long arg2 = getarg(machine, start, modes, 2);
  setarg(machine, start, modes, 3, arg1 * arg2);
}

static void prog_input(struct IntCodeMachine *machine, size_t start, long modes, long input) {
  setarg(machine, start, modes, 1, input);
}

static long prog_output(struct IntCodeMachine *machine, size_t start, long modes) {
  long o = getarg(machine, start, modes, 1);
  return o;
}

static long prog_jumpif(struct IntCodeMachine *machine, size_t start, long modes, bool what) {
  int arg1 = getarg(machine, start, modes, 1);
  bool cond = arg1 != 0;

  long pc;
  if ((cond && what) || (!cond && !what)) {
    pc = getarg(machine, start, modes, 2);
  } else {
    pc = start + 3;
  }
  return pc;
}

static void prog_lessthan(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg1 = getarg(machine, start, modes, 1);
  long arg2 = getarg(machine, start, modes, 2);

  if (arg1 < arg2) {
    setarg(machine, start, modes, 3, 1);
  } else {
    setarg(machine, start, modes, 3, 0);
  }
}

static void prog_equals(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg1 = getarg(machine, start, modes, 1);
  long arg2 = getarg(machine, start, modes, 2);

  if (arg1 == arg2) {
    setarg(machine, start, modes, 3, 1);
  } else {
    setarg(machine, start, modes, 3, 0);
  }
}

static void prog_adjust_relative_base(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg = getarg(machine, start, modes, 1);
  machine->relative_base += arg;
}

void machine_run(struct IntCodeMachine *machine) {
  if (!machine->running) {
    machine->running = true;
    machine->pc = 0;
  }

  while (machine->pc < machine->program_size) {
    long opcode = machine->program[machine->pc] % 100;
    long modes = machine->program[machine->pc] / 100;

    switch (opcode) {
    case 1:
      prog_sum(machine, machine->pc, modes);
      machine->pc += 4;
      break;
    case 2:
      prog_mult(machine, machine->pc, modes);
      machine->pc += 4;
      break;
    case 3:
      if (machine->has_input) {
        machine->has_input = false;
        prog_input(machine, machine->pc, modes, machine->input);
        machine->pc += 2;
        break;
      } else {
        return;
      }
    case 4:
#ifdef DEBUG
      if (machine->has_output) {
        fprintf(
            stderr,
            "Warning: Machine needs to output data but last output was not acknowleged. Last output will be lost.\n");
      }
#endif
      machine->has_output = true;
      machine->output = prog_output(machine, machine->pc, modes);
      machine->pc += 2;
      return;
    case 5:
      machine->pc = prog_jumpif(machine, machine->pc, modes, true);
      break;
    case 6:
      machine->pc = prog_jumpif(machine, machine->pc, modes, false);
      break;
    case 7:
      prog_lessthan(machine, machine->pc, modes);
      machine->pc += 4;
      break;
    case 8:
      prog_equals(machine, machine->pc, modes);
      machine->pc += 4;
      break;
    case 9:
      prog_adjust_relative_base(machine, machine->pc, modes);
      machine->pc += 2;
      break;
    case 99:
      machine->running = false;
      return;
#ifdef DEBUG
    default:
      fprintf(stderr, "Warning: Ignoring unexpected opcode: %ld (modes: %ld)\n", opcode, modes);
#endif
    }
  }

#ifdef DEBUG
  fprintf(stderr, "Error: Execution extended beyond program size. Halting.\n");
#endif
}
