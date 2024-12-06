#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct IntCodeMachine {
  long *program;
  size_t program_size;
  size_t relative_base;

  bool running;
  size_t pc;

  bool has_input;
  long input;
  bool has_output;
  long output;
};

static long *parse_program(const char *const input, size_t *const size) {
  long *program = calloc(32, sizeof(*program));
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
        program = realloc(program, sizeof(*program) * program_size * 2);
        memset(program + program_size, 0, program_size * sizeof(*program));
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

  *size = program_size;

  return program;
}

static void machine_init(struct IntCodeMachine *const machine, const char *const input) {
  machine->program = parse_program(input, &machine->program_size);
  machine->relative_base = 0;

  machine->running = false;
  machine->has_input = false;
  machine->has_output = false;
}

static bool machine_recv_output(struct IntCodeMachine *const machine, long *output) {
  if (machine->has_output) {
    machine->has_output = false;
    *output = machine->output;
    return true;
  } else {
    return false;
  }
}

static bool machine_send_input(struct IntCodeMachine *const machine, long input) {
  if (machine->has_input) {
    return false;
  } else {
    machine->has_input = true;
    machine->input = input;
    return true;
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
  if (index > 1 << 20) {
    fprintf(stderr, "I'm afraid I can't let you do that, Dave. (Refusing to allocate memory to access index %lu)",
            index);
    abort();
  }

  bool modified_size = false;
  size_t size = machine->program_size;

  while (index > size) {
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
    DBG("reading from address %ld directly", arg);
    return read_memory(machine, arg);
  case 1:
    DBG("immediate value %ld used instead of a read", arg);
    return arg;
  case 2:
    DBG("reading from address %ld indirectly (%ld + %ld)", machine->relative_base + arg, machine->relative_base, arg);
    return read_memory(machine, machine->relative_base + arg);
  default:
    FAIL("Invalid mode: %ld", mode);
  }
}

static void set_argument(struct IntCodeMachine *machine, size_t i, long mode, long value) {
  long arg = read_memory(machine, i);

  switch (mode) {
  case 0:
    DBG("writing to address %ld directly", arg);
    write_memory(machine, arg, value);
    break;
  case 1:
    FAIL("Attempt to write with immediate mode");
  case 2:
    DBG("writing to address %ld indirectly (%ld + %ld)", machine->relative_base + arg, machine->relative_base, arg);
    write_memory(machine, machine->relative_base + arg, value);
    break;
  default:
    FAIL("Invalid mode: %ld", mode);
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
  DBG("sum: %ld + %ld", arg1, arg2);
  setarg(machine, start, modes, 3, arg1 + arg2);
}

static void prog_mult(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg1 = getarg(machine, start, modes, 1);
  long arg2 = getarg(machine, start, modes, 2);
  DBG("mult: %ld * %ld", arg1, arg2);
  setarg(machine, start, modes, 3, arg1 * arg2);
}

static void prog_input(struct IntCodeMachine *machine, size_t start, long modes, long input) {
  DBG("input: %ld", input);
  setarg(machine, start, modes, 1, input);
}

static long prog_output(struct IntCodeMachine *machine, size_t start, long modes) {
  long o = getarg(machine, start, modes, 1);
  DBG("output: %ld", o);
  return o;
}

static long prog_jumpif(struct IntCodeMachine *machine, size_t start, long modes, bool what) {
  int arg1 = getarg(machine, start, modes, 1);
  bool cond = arg1 != 0;

  long pc;
  if ((cond && what) || (!cond && !what)) {
    pc = getarg(machine, start, modes, 2);
    DBG("jumpif%s resulting in a jump to: %ld", what ? "true" : "false", pc);
  } else {
    pc = start + 3;
    DBG("jumpif%s resulting in no jump", what ? "true" : "false");
  }
  return pc;
}

static void prog_lessthan(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg1 = getarg(machine, start, modes, 1);
  long arg2 = getarg(machine, start, modes, 2);

  if (arg1 < arg2) {
    DBG("lessthan true: %ld < %ld", arg1, arg2);
    setarg(machine, start, modes, 3, 1);
  } else {
    DBG("lessthan false: %ld >= %ld", arg1, arg2);
    setarg(machine, start, modes, 3, 0);
  }
}

static void prog_equals(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg1 = getarg(machine, start, modes, 1);
  long arg2 = getarg(machine, start, modes, 2);

  if (arg1 == arg2) {
    DBG("equals true: %ld == %ld", arg1, arg2);
    setarg(machine, start, modes, 3, 1);
  } else {
    DBG("equals false: %ld != %ld", arg1, arg2);
    setarg(machine, start, modes, 3, 0);
  }
}

static void prog_adjust_relative_base(struct IntCodeMachine *machine, size_t start, long modes) {
  long arg = getarg(machine, start, modes, 1);
  DBG("updated relative base += %ld", arg);
  machine->relative_base += arg;
}

static void machine_run(struct IntCodeMachine *machine) {
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
      ASSERT(!machine->has_output, "Did not acknowledge machine output");
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
    default:
      FAIL("Found an unexpected opcode: %ld (modes: %ld)", opcode, modes);
    }
  }

  FAIL("RAN OUT OF PROGRAM TO EXECUTE!");
}

static void solution(struct IntCodeMachine *machine, const int gridsize, int *const grid, int *const paintedgrid,
                     const int startingcell) {
  int x = 0, y = 0;
  enum { NORTH, EAST, SOUTH, WEST } direction = NORTH;

  grid[gridsize / 2 * gridsize + gridsize / 2] = startingcell;

  machine_run(machine);
  while (machine->running) {
    int newx = x + gridsize / 2;
    int newy = y + gridsize / 2;

    int color = grid[newy * gridsize + newx];

    ASSERT(machine_send_input(machine, color), "Machine should accept input at this point.");
    machine_run(machine);

    long new_color;
    ASSERT(machine_recv_output(machine, &new_color), "Machine should have output at this point.");

    grid[newy * gridsize + newx] = new_color;
    paintedgrid[newy * gridsize + newx] = 1;

    machine_run(machine);

    long direction_change;
    ASSERT(machine_recv_output(machine, &direction_change), "Machine should have output at this point too.");

    if (direction_change == 0) {
      if (direction == NORTH) {
        direction = WEST;
      } else {
        direction--;
      }
    } else {
      if (direction == WEST) {
        direction = NORTH;
      } else {
        direction++;
      }
    }

    switch (direction) {
    case NORTH:
      y++;
      break;
    case EAST:
      x++;
      break;
    case SOUTH:
      y--;
      break;
    case WEST:
      x--;
      break;
    default:
      FAIL("Direction has unexpected value %d", direction);
    }

    ASSERT(x + gridsize / 2 >= 0, "X has become too small");
    ASSERT(y + gridsize / 2 >= 0, "Y has become too small");
    ASSERT(x + gridsize / 2 < gridsize, "X has become too big");
    ASSERT(y + gridsize / 2 < gridsize, "Y has become too big");

    machine_run(machine);
  }
}

static void solution1(const char *const input, char *const output) {
  struct IntCodeMachine machine;
  machine_init(&machine, input);

  const int gridsize = 1 << 8;

  int *grid = malloc(sizeof(int) * gridsize * gridsize);
  for (int i = 0; i < gridsize * gridsize; i++) {
    grid[i] = 0;
  }

  int *paintedgrid = malloc(sizeof(int) * gridsize * gridsize);
  for (int i = 0; i < gridsize * gridsize; i++) {
    paintedgrid[i] = 0;
  }

  solution(&machine, gridsize, grid, paintedgrid, 0);

  int count = 0;
  for (int i = 0; i < gridsize * gridsize; i++) {
    if (paintedgrid[i] == 1) {
      count++;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
  free(machine.program);
  free(grid);
  free(paintedgrid);
}

static void solution2(const char *const input, char *const output) {
  struct IntCodeMachine machine;
  machine_init(&machine, input);

  const int gridsize = 1 << 8;

  int *grid = malloc(sizeof(int) * gridsize * gridsize);
  for (int i = 0; i < gridsize * gridsize; i++) {
    grid[i] = 0;
  }

  int *paintedgrid = malloc(sizeof(int) * gridsize * gridsize);
  for (int i = 0; i < gridsize * gridsize; i++) {
    paintedgrid[i] = 0;
  }

  solution(&machine, gridsize, grid, paintedgrid, 1);

  int startx, starty, endx, endy;

  starty = 0;
  for (int j = 0; j < gridsize; j++) {
    bool row_empty = true;
    for (int i = 0; i < gridsize; i++) {
      int n = grid[j * gridsize + i];
      if (n == 1) {
        row_empty = false;
        break;
      }
    }

    if (!row_empty) {
      starty = j;
      break;
    }
  }

  int tentative_endy = starty;
  bool in_empty_space = false;
  for (int j = starty; j < gridsize; j++) {
    bool row_empty = true;
    for (int i = 0; i < gridsize; i++) {
      int n = grid[j * gridsize + i];
      if (n == 1) {
        row_empty = false;
        break;
      }
    }

    if (in_empty_space) {
      if (!row_empty) {
        in_empty_space = false;
      }
    } else {
      if (row_empty) {
        tentative_endy = j;
        in_empty_space = true;
      }
    }
  }
  endy = tentative_endy;

  startx = gridsize - 1;
  endx = 0;
  for (int j = starty; j < endy; j++) {
    int firstfull = -1, lastfull = -1;
    bool empty_row = true;
    for (int i = 0; i < gridsize; i++) {
      int n = grid[j * gridsize + i];
      if (n == 1) {
        empty_row = false;
        lastfull = i;
        if (firstfull == -1) {
          firstfull = i;
        }
      }
    }

    if (!empty_row) {
      if (firstfull < startx) {
        startx = firstfull;
      }
      if (lastfull > endx) {
        endx = lastfull;
      }
    }
  }
  endx += 1;

  size_t buffer_cap = 50;
  char *buffer = malloc(sizeof(*buffer) * buffer_cap);
  size_t buffer_len = 0;
  size_t buffer_width = 0;
  size_t buffer_height = 0;

  for (int j = endy; j >= starty; j--) { // It's inverted :^)
    for (int i = startx; i < endx; i++) {
      if (buffer_len >= buffer_cap) {
        buffer_cap *= 2;
        buffer = realloc(buffer, sizeof(*buffer) * buffer_cap);
      }
      int n = grid[j * gridsize + i];
      if (n == 0) {
        buffer[buffer_len++] = ' ';
      } else if (n == 1) {
        buffer[buffer_len++] = '#';
      } else {
        buffer[buffer_len++] = '?';
      }
    }
    if (buffer_height == 0) {
      buffer_width = buffer_len;
    }
    buffer_height++;
  }

#ifdef DEBUG
  for (size_t j = 1; j < buffer_height; j++) {
    for (size_t i = 0; i < buffer_width; i++) {
      fputc(buffer[j * buffer_width + i], stderr);
    }
    fputc('\n', stderr);
  }
#endif

  char *result = aoc_ocr(buffer + buffer_width, buffer_width, buffer_height - 1);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", result);
  free(machine.program);
  free(grid);
  free(paintedgrid);
  free(buffer);
  free(result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
