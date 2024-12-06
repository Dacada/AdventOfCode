#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NPROGS (5 * 5 * 5 * 5 * 5 * 5)

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

    if (isspace(c)) {
      continue;
    } else if (c == ',') {
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
  // DBG("[%d] = %d + %d", res_index, arg1, arg2);
}

static void prog_mult(int *const program, int start, int modes) {
  int arg1 = getarg(program, start, modes, 1);
  int arg2 = getarg(program, start, modes, 2);
  int res_index = program[start + 3];
  program[res_index] = arg1 * arg2;
  // DBG("[%d] = %d * %d", res_index, arg1, arg2);
}

static void prog_input(int *const program, int start, int modes, int input) {
  (void)modes;
  int res_index = program[start + 1];
  program[res_index] = input;
  // DBG("[%d] = input", res_index);
}

static int prog_output(int *const program, int start, int modes) {
  int output = getarg(program, start, modes, 1);
  // DBG("output %d", output);
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

struct IntCodeMachine {
  int *program;
  size_t program_size;

  bool running;
  size_t pc;

  bool has_input;
  int input;
  bool has_output;
  int output;
};

static void run_intcode(struct IntCodeMachine *machine) {
  if (!machine->running) {
    machine->running = true;
    machine->pc = 0;
  }

  while (machine->pc < machine->program_size) {
    int opcode = machine->program[machine->pc] % 100;
    int modes = machine->program[machine->pc] / 100;

    switch (opcode) {
    case 1:
      prog_sum(machine->program, machine->pc, modes);
      machine->pc += 4;
      break;
    case 2:
      prog_mult(machine->program, machine->pc, modes);
      machine->pc += 4;
      break;
    case 3:
      if (machine->has_input) {
        // DBG("Input - Have input, continuing");
        machine->has_input = false;
        prog_input(machine->program, machine->pc, modes, machine->input);
        machine->pc += 2;
        break;
      } else {
        // DBG("Input - No input, returning");
        return;
      }
    case 4:
      ASSERT(!machine->has_output, "Did not acknowledge machine output");
      // DBG("Output - returning with output");

      machine->has_output = true;
      machine->output = prog_output(machine->program, machine->pc, modes);
      machine->pc += 2;
      return;
    case 5:
      machine->pc = prog_jumpif(machine->program, machine->pc, modes, true);
      break;
    case 6:
      machine->pc = prog_jumpif(machine->program, machine->pc, modes, false);
      break;
    case 7:
      prog_lessthan(machine->program, machine->pc, modes);
      machine->pc += 4;
      break;
    case 8:
      prog_equals(machine->program, machine->pc, modes);
      machine->pc += 4;
      break;
    case 99:
      machine->running = false;
      return;
    default:
      FAIL("Found an unexpected opcode: %d (modes: %d)", opcode, modes);
    }
  }
}

struct permutationArgs {
  int maxSignal;
  int *program;
  size_t program_size;
};

static void run_phase_configuration_1(int *phases, void *args) {
  struct permutationArgs *pargs = args;

  int *programs[5];
  for (int i = 0; i < 5; i++) {
    int *program = malloc(sizeof(int) * pargs->program_size);
    memcpy(program, pargs->program, sizeof(int) * pargs->program_size);
    programs[i] = program;
  }

  DBG("Phases: %d,%d,%d,%d,%d", phases[0], phases[1], phases[2], phases[3], phases[4]);

  struct IntCodeMachine machine;
  int signal = 0;
  for (int i = 0; i < 5; i++) {
    machine.program = programs[i];
    machine.program_size = pargs->program_size;
    machine.running = false;
    machine.has_input = true;
    machine.input = phases[i];
    machine.has_output = false;

    run_intcode(&machine);
    ASSERT(!machine.has_input, "Ran program but did not consume first input");
    ASSERT(machine.running, "Ran program but exited immediately");
    ASSERT(!machine.has_output, "Ran program and immediately got some output");

    machine.has_input = true;
    machine.input = signal;

    run_intcode(&machine);
    ASSERT(machine.has_output, "Ran program but got no output");
    ASSERT(!machine.has_input, "Ran program but expects more input");
    ASSERT(machine.running, "Ran program but machine stopped running after giving output");

    machine.has_output = false;
    signal = machine.output;

    run_intcode(&machine);
    ASSERT(!machine.running, "Ran program but machine didn't finish");
  }

  if (signal > pargs->maxSignal) {
    DBG("Signal improved from %d to %d", pargs->maxSignal, signal);
    pargs->maxSignal = signal;
  }

  for (int i = 0; i < 5; i++) {
    free(programs[i]);
  }
}

static void run_phase_configuration_2(int *phases, void *args) {
  struct permutationArgs *pargs = args;

  int *programs[5];
  for (int i = 0; i < 5; i++) {
    int *program = malloc(sizeof(int) * pargs->program_size);
    memcpy(program, pargs->program, sizeof(int) * pargs->program_size);
    programs[i] = program;
  }

  DBG("Phases: %d,%d,%d,%d,%d", phases[0], phases[1], phases[2], phases[3], phases[4]);

  struct IntCodeMachine machines[5];
  for (int i = 0; i < 5; i++) {
    machines[i].program = programs[i];
    machines[i].program_size = pargs->program_size;
    machines[i].running = false;
    machines[i].has_input = true;
    machines[i].input = phases[i];
    machines[i].has_output = false;

    run_intcode(&machines[i]);
    ASSERT(!machines[i].has_output, "First run of program already has output");
    ASSERT(!machines[i].has_input, "First run of program but machine did not consume input");
    ASSERT(machines[i].running, "First run of program but machines already finished running");
  }
  machines[0].input = 0;
  machines[0].has_input = true;

  bool finished = false;
  while (!finished) {
    for (int i = 0; i < 5; i++) {
      run_intcode(&machines[i]);

      if (!machines[i].running) {
        finished = true;
      } else {
        ASSERT(machines[i].has_output, "Ran program but machine has no output");
        ASSERT(!machines[(i + 1) % 5].has_input, "Next machine still has pending input");

        machines[(i + 1) % 5].input = machines[i].output;
        machines[i].has_output = false;
        machines[(i + 1) % 5].has_input = true;
      }

      if (finished) {
        ASSERT(!machines[i].running, "A machine finished running but not all did");
      }
    }
  }

  int signal = machines[4].output;
  if (signal > pargs->maxSignal) {
    DBG("Signal improved from %d to %d", pargs->maxSignal, signal);
    pargs->maxSignal = signal;
  }

  for (int i = 0; i < 5; i++) {
    free(programs[i]);
  }
}

static void solution(const char *const input, char *const output, int phases[5],
                     void (*run_phase_configuration)(int *, void *)) {
  size_t program_size;
  int *program = parse_program(input, &program_size);
  if (program == NULL) {
    snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
    return;
  }

  struct permutationArgs args = {.maxSignal = 0, .program = program, .program_size = program_size};

  aoc_permute(phases, 5, run_phase_configuration, &args);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", args.maxSignal);
  free(program);
}

static void solution1(const char *const input, char *const output) {
  int phases[5] = {0, 1, 2, 3, 4};
  solution(input, output, phases, run_phase_configuration_1);
}

static void solution2(const char *const input, char *const output) {
  int phases[5] = {5, 6, 7, 8, 9};
  solution(input, output, phases, run_phase_configuration_2);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
