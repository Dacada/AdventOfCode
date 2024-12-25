#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>

enum opcode {
  CPY,
  INC,
  DEC,
  JNZ,
  TGL,

  // "unofficial" additions
  MUL,
  ADD,
};

enum reg {
  REG_A,
  REG_B,
  REG_C,
  REG_D,
  REG_INVALID = 99,
};

struct operand {
  bool is_reg;
  union {
    enum reg reg;
    long val;
  } value;
};

struct instr {
  enum opcode opcode;
  struct operand op1;
  struct operand op2;
};

struct machine {
  size_t pc;
  long regs[4];
  struct instr *prog_mem;
  size_t prog_mem_len;
};

static long get_operand_value(const struct machine *const machine, const struct operand *const operand) {
  if (operand->is_reg) {
    return machine->regs[operand->value.reg];
  } else {
    return operand->value.val;
  }
}

static enum reg get_register(const struct operand *const operand) {
  if (operand->is_reg) {
    return operand->value.reg;
  } else {
    return REG_INVALID;
  }
}

static void toggle_instr(struct instr *instr) {
  switch (instr->opcode) {
  case INC:
    instr->opcode = DEC;
    break;
  case DEC:
  case TGL:
    instr->opcode = INC;
    break;

  case JNZ:
    instr->opcode = CPY;
    break;
  case CPY:
  case ADD:
  case MUL:
    instr->opcode = JNZ;
    break;
  }
}

static void run(struct machine *const machine) {
  while (machine->pc < machine->prog_mem_len) {
    struct instr *instr = machine->prog_mem + machine->pc;
    switch (instr->opcode) {
    case CPY: {
      long val = get_operand_value(machine, &instr->op1);
      enum reg dest = get_register(&instr->op2);
      if (dest != REG_INVALID) {
        machine->regs[dest] = val;
      }
    } break;
    case INC: {
      enum reg target = get_register(&instr->op1);
      if (target != REG_INVALID) {
        machine->regs[target]++;
      }
    } break;
    case DEC: {
      enum reg target = get_register(&instr->op1);
      if (target != REG_INVALID) {
        machine->regs[target]--;
      }
    } break;
    case JNZ: {
      long tst = get_operand_value(machine, &instr->op1);
      if (tst != 0) {
        long jmp = get_operand_value(machine, &instr->op2);
        machine->pc += jmp - 1;
      }
    } break;
    case TGL: {
      long target = machine->pc + get_operand_value(machine, &instr->op1);
      if (target >= 0 && (unsigned long)target < machine->prog_mem_len) {
        struct instr *target_instr = &machine->prog_mem[target];
        toggle_instr(target_instr);
      }
    } break;
    case ADD: {
      enum reg target = get_register(&instr->op2);
      long op1 = get_operand_value(machine, &instr->op1);
      long op2 = get_operand_value(machine, &instr->op2);
      machine->regs[target] = op1 + op2;
    } break;
    case MUL: {
      enum reg target = get_register(&instr->op2);
      long op1 = get_operand_value(machine, &instr->op1);
      long op2 = get_operand_value(machine, &instr->op2);
      machine->regs[target] = op1 * op2;
    } break;
    default:
      FAIL("illegal instruction");
    }

    machine->pc++;
  }
}

static void parse_int_operand(const char **const input, struct operand *const operand) {
  operand->is_reg = false;

  bool neg = **input == '-';
  if (neg) {
    *input += 1;
  }

  long val = 0;
  while (isdigit(**input)) {
    val *= 10;
    val += **input - '0';
    *input += 1;
  }

  if (neg) {
    operand->value.val = -val;
  } else {
    operand->value.val = val;
  }
}

static void parse_reg_operand(const char **const input, struct operand *const operand) {
  operand->is_reg = true;

  switch (**input) {
  case 'a':
    operand->value.reg = REG_A;
    break;
  case 'b':
    operand->value.reg = REG_B;
    break;
  case 'c':
    operand->value.reg = REG_C;
    break;
  case 'd':
    operand->value.reg = REG_D;
    break;
  default:
    FAIL("parse error");
  }

  *input += 1;
}

static void parse_operand(const char **const input, struct operand *const operand) {
  if (isdigit(**input) || **input == '-') {
    parse_int_operand(input, operand);
  } else {
    parse_reg_operand(input, operand);
  }
}

static void parse_instr(const char **const input, struct instr *const instr, int operands, enum opcode opcode) {
  instr->opcode = opcode;
  *input += 4;

  char sep;
  if (operands == 1) {
    sep = '\n';
  } else if (operands == 2) {
    sep = ' ';
  } else {
    FAIL("parse error");
  }

  parse_operand(input, &instr->op1);
  ASSERT(**input == sep, "parse error");
  *input += 1;

  if (operands == 1) {
    return;
  }

  parse_operand(input, &instr->op2);
  ASSERT(**input == '\n', "parse error");
  *input += 1;
}

static void parse_line(const char **const input, struct machine *const machine, size_t *prog_mem_cap) {
  bool resize = false;
  while (machine->prog_mem_len >= *prog_mem_cap) {
    *prog_mem_cap *= 2;
    resize = true;
  }
  if (resize) {
    machine->prog_mem = realloc(machine->prog_mem, sizeof(*machine->prog_mem) * (*prog_mem_cap));
  }
  struct instr *instr = machine->prog_mem + machine->prog_mem_len;
  machine->prog_mem_len += 1;

  switch (**input) {
  case 'c':
    parse_instr(input, instr, 2, CPY);
    break;
  case 'i':
    parse_instr(input, instr, 1, INC);
    break;
  case 'd':
    parse_instr(input, instr, 1, DEC);
    break;
  case 'j':
    parse_instr(input, instr, 2, JNZ);
    break;
  case 't':
    parse_instr(input, instr, 1, TGL);
    break;
  default:
    FAIL("parse error");
  }
}

static void parse(const char *input, struct machine *const machine) {
  machine->pc = 0;
  machine->regs[REG_A] = 0;
  machine->regs[REG_B] = 0;
  machine->regs[REG_C] = 0;
  machine->regs[REG_D] = 0;

  size_t prog_mem_cap = 16;
  machine->prog_mem = malloc(sizeof(*machine->prog_mem) * prog_mem_cap);
  machine->prog_mem_len = 0;

  while (*input != '\0') {
    parse_line(&input, machine, &prog_mem_cap);
    if (*input == '\n') {
      input += 1;
    }
  }
}

static void free_machine(struct machine *const machine) { free(machine->prog_mem); }

static void solution(const char *const input, char *const output, long regA_val) {
  struct machine machine;
  parse(input, &machine);

  for (unsigned i = 0; i < machine.prog_mem_len - 5; i++) {
    struct instr *instr0 = &machine.prog_mem[i];
    struct instr *instr1 = &machine.prog_mem[i + 1];
    struct instr *instr2 = &machine.prog_mem[i + 2];
    struct instr *instr3 = &machine.prog_mem[i + 3];
    struct instr *instr4 = &machine.prog_mem[i + 4];
    struct instr *instr5 = &machine.prog_mem[i + 5];

    /* 0 cpy b c */
    /* 1 inc a */
    /* 2 dec c */
    /* 3 jnz c -2 */
    /* 4 dec d */
    /* 5 jnz d -5 */

    if (instr0->opcode != CPY || instr1->opcode != INC || instr2->opcode != DEC || instr3->opcode != JNZ ||
        instr4->opcode != DEC || instr5->opcode != JNZ) {
      continue;
    }

    if (!instr0->op1.is_reg || !instr0->op2.is_reg || !instr1->op1.is_reg || !instr2->op1.is_reg ||
        !instr3->op1.is_reg || instr3->op2.is_reg || !instr4->op1.is_reg || !instr5->op1.is_reg || instr5->op2.is_reg) {
      continue;
    }

    enum reg regA = instr1->op1.value.reg;
    enum reg regB = instr0->op1.value.reg;
    enum reg regC = instr0->op2.value.reg;
    enum reg regD = instr4->op1.value.reg;

    /* 0 mul b d */
    /* 1 add d a */
    /* 2 cpy 0 c */
    /* 3 cpy 0 d */
    /* 4 cpy 0 c */
    /* 5 cpy 0 d */

    instr0->opcode = MUL;
    instr0->op1.is_reg = true;
    instr0->op1.value.reg = regB;
    instr0->op2.is_reg = true;
    instr0->op2.value.reg = regD;

    instr1->opcode = ADD;
    instr1->op1.is_reg = true;
    instr1->op1.value.reg = regD;
    instr1->op2.is_reg = true;
    instr1->op2.value.reg = regA;

    instr2->opcode = CPY;
    instr2->op1.is_reg = false;
    instr2->op1.value.val = 0;
    instr2->op2.is_reg = true;
    instr2->op2.value.reg = regC;

    instr3->opcode = CPY;
    instr3->op1.is_reg = false;
    instr3->op1.value.val = 0;
    instr3->op2.is_reg = true;
    instr3->op2.value.reg = regD;

    *instr4 = *instr2;
    *instr5 = *instr3;

    DBG("optimized a multiplication");
  }

  machine.regs[REG_A] = regA_val;
  run(&machine);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", machine.regs[REG_A]);
  free_machine(&machine);
}

static void solution1(const char *const input, char *const output) { solution(input, output, 7); }

static void solution2(const char *const input, char *const output) { solution(input, output, 12); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
