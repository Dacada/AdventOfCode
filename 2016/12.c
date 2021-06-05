#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

enum opcode {
        CPY,
        INC,
        DEC,
        JNZ,
};

enum reg {
        REG_A,
        REG_B,
        REG_C,
        REG_D,
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

static long get_operand_value(const struct machine *const machine,
                              const struct operand *const operand) {
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
                FAIL("invalid operand");
        }
}

static void run(struct machine *const machine) {
        while (machine->pc < machine->prog_mem_len) {
                struct instr *instr = machine->prog_mem + machine->pc;
                switch (instr->opcode) {
                case CPY: {
                        long val = get_operand_value(machine, &instr->op1);
                        enum reg dest = get_register(&instr->op2);
                        machine->regs[dest] = val;
                }
                        break;
                case INC: {
                        enum reg target = get_register(&instr->op1);
                        machine->regs[target]++;
                }
                        break;
                case DEC: {
                        enum reg target = get_register(&instr->op1);
                        machine->regs[target]--;
                }
                        break;
                case JNZ: {
                        long tst = get_operand_value(machine, &instr->op1);
                        if (tst != 0) {
                                long jmp = get_operand_value(machine, &instr->op2);
                                machine->pc += jmp - 1;
                        }
                }
                        break;
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
                machine->prog_mem = realloc(machine->prog_mem,
                                            sizeof(*machine->prog_mem)*(*prog_mem_cap));
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
        machine->prog_mem = malloc(sizeof(*machine->prog_mem)*prog_mem_cap);
        machine->prog_mem_len = 0;

        while (*input != '\0') {
                parse_line(&input, machine, &prog_mem_cap);
                if (*input == '\n') {
                        input += 1;
                }
        }
}

static void free_machine(struct machine *const machine) {
        free(machine->prog_mem);
}

static void solution(const char *const input, char *const output, long regC) {
        struct machine machine;
        parse(input, &machine);
        machine.regs[REG_C] = regC;
        run(&machine);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", machine.regs[REG_A]);
        free_machine(&machine);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 0);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 1);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
