#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

enum instr {
  INSTR_ADV = 0,
  INSTR_BXL = 1,
  INSTR_BST = 2,
  INSTR_JNZ = 3,
  INSTR_BXC = 4,
  INSTR_OUT = 5,
  INSTR_BDV = 6,
  INSTR_CDV = 7,
};

struct machine {
  long reg_a;
  long reg_b;
  long reg_c;
  int pc;
  int ninstrs;
  enum instr *instrs;
};

static struct machine parse_input(const char *input) {
  struct machine machine;
  machine.pc = 0;

  const char *txt = "Register A: ";
  aoc_expect_text(&input, txt, strlen(txt));
  machine.reg_a = aoc_parse_long(&input);

  txt = "\nRegister B: ";
  aoc_expect_text(&input, txt, strlen(txt));
  machine.reg_b = aoc_parse_long(&input);

  txt = "\nRegister C: ";
  aoc_expect_text(&input, txt, strlen(txt));
  machine.reg_c = aoc_parse_long(&input);

  txt = "\n\nProgram: ";
  aoc_expect_text(&input, txt, strlen(txt));
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(enum instr), 8);
  while (*input != '\0') {
    int *n = aoc_dynarr_grow(&arr, 1);
    ASSERT(isdigit(*input), "parse error");
    *n = *input - '0';
    input += 1;
    if (*input == ',' || *input == '\n') {
      input += 1;
    }
  }
  machine.instrs = arr.data;
  machine.ninstrs = arr.len;

  return machine;
}

static char *comma_separated(int *elements, int len) {
  if (len == 0) {
    char *str = aoc_malloc(sizeof(char));
    *str = '\0';
    return str;
  }
  char *str = aoc_malloc(sizeof(char) * len * 2);
  for (int i = 0; i < len; i++) {
    str[i * 2] = elements[i] + '0';
    str[i * 2 + 1] = ',';
  }
  str[len * 2 - 1] = '\0';
  return str;
}

static bool is_combo(enum instr instr) {
  return instr == INSTR_ADV || instr == INSTR_BST || instr == INSTR_OUT || instr == INSTR_BDV || instr == INSTR_CDV;
}

static int get_combo(enum instr oper, const struct machine *machine) {
  if (oper >= 0 && oper <= 3) {
    return oper;
  }
  if (oper == 4) {
    return machine->reg_a;
  }
  if (oper == 5) {
    return machine->reg_b;
  }
  if (oper == 6) {
    return machine->reg_c;
  }
  FAIL("invalid program");
}

static void run(struct machine *machine, struct aoc_dynarr *output) {
  for (;;) {
    if (machine->pc + 1 >= machine->ninstrs) {
      break;
    }

    enum instr instr = machine->instrs[machine->pc];
    enum instr oper = machine->instrs[machine->pc + 1];

    int arg;
    if (is_combo(instr)) {
      arg = get_combo(oper, machine);
    } else {
      arg = oper;
    }

    long *res;
    int *out;
    switch (instr) {

    // I thought it'd be funny to implement the *DV instructions with goto:
    case INSTR_ADV:
      res = &machine->reg_a;
    DV:
      *res = machine->reg_a / (1 << arg);
      break;
    case INSTR_BDV:
      res = &machine->reg_b;
      goto DV;
    case INSTR_CDV:
      res = &machine->reg_c;
      goto DV;

    case INSTR_BXL:
      machine->reg_b = machine->reg_b ^ arg;
      break;

    case INSTR_BST:
      machine->reg_b = aoc_modulo_long(arg, 8);
      break;

    case INSTR_JNZ:
      if (machine->reg_a != 0) {
        machine->pc = arg - 2;
      }
      break;

    case INSTR_BXC:
      machine->reg_b = machine->reg_b ^ machine->reg_c;
      break;

    case INSTR_OUT:
      out = aoc_dynarr_grow(output, 1);
      *out = (int)(aoc_modulo_long(arg, 8));
      break;

    default:
      FAIL("invalid instruction: %d", instr);
    }

    machine->pc += 2;
  }
}

static void solution1(const char *input, char *const output) {
  struct machine machine = parse_input(input);

  {
    DBG("Register A: %ld", machine.reg_a);
    DBG("Register B: %ld", machine.reg_b);
    DBG("Register C: %ld", machine.reg_c);
    DBG(" ");
    char *str = comma_separated((int *)machine.instrs, machine.ninstrs);
    DBG("Program: %s", str);
    free(str);
  }

  struct aoc_dynarr res;
  aoc_dynarr_init(&res, sizeof(int), 8);
  run(&machine, &res);
  char *str = comma_separated((int *)res.data, res.len);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", str);
  free(machine.instrs);
  aoc_dynarr_free(&res);
  free(str);
}

static void run_for_a(struct machine machine, struct aoc_dynarr *res, long a) {
  aoc_dynarr_truncate(res);
  machine.reg_a = a;
  run(&machine, res);

#ifdef DEBUG
  char *str = comma_separated((int *)res->data, res->len);
  DBG("A=%ld: %s", a, str);
  free(str);
#endif
}

static bool array_different(const unsigned *prev, const int *curr, int start, int len) {
  for (int i = start; i < len; i++) {
    if ((int)prev[i] != curr[i]) {
      return true;
    }
  }
  return false;
}

static void solution2(const char *input, char *const output) {
  struct machine machine = parse_input(input);
  struct aoc_dynarr res;
  aoc_dynarr_init(&res, sizeof(int), 8);

  // Key assumption: all inputs follow these patterns.
  //
  // The length of an output can be easily predicted as a function of A
  // 8^(len-1) <= A <= 8^(len)
  // With powers of 8 being easily convertible to powers of 2
  //
  // The first digit changes when incrementing A by 1
  // The second when incrementing it by 8
  // The third when incrementing by 16
  // The nth when incrementing by 8^(n-1)
  //
  // I start from the last digits but it would probably work them same starting
  // from the first.

  long a = (1L << 45L);
  run_for_a(machine, &res, a);

  long increment = 1L << (3 * (machine.ninstrs - 1));

  for (int i = machine.ninstrs - 1; i >= 0; i--) {
    DBG("looking for %d", machine.instrs[i]);
    while (AOC_DYNARR_IDX(res, i, int) != (int)machine.instrs[i]) {
      a += increment;

      run_for_a(machine, &res, a);

      if (array_different(machine.instrs, res.data, i + 1, res.len)) {
        DBG("backtrack");
        increment *= 8;
        i++;
      }
    }
    DBG("DIGIT %d LOCKED IN!!", i);
    increment /= 8;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", a);
  free(machine.instrs);
  aoc_dynarr_free(&res);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
