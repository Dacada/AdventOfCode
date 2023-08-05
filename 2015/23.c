#include <aoclib.h>
#include <stdio.h>

enum instruction_type {
        INSTR_HLF,
        INSTR_TPL,
        INSTR_INC,
        INSTR_JMP,
        INSTR_JIE,
        INSTR_JIO,
};

enum register_type {
        REG_A,
        REG_B,
};

struct instruction {
        enum instruction_type instr;
        enum register_type reg;
        int offset;
};

static const char *parse_instr(const char *input, struct instruction *instruction, bool *has_reg, bool *has_off) {
        switch (*input) {
        case 'h':
                instruction->instr = INSTR_HLF;
                *has_reg = true;
                *has_off = false;
                return input+3;
        case 't':
                instruction->instr = INSTR_TPL;
                *has_reg = true;
                *has_off = false;
                return input+3;
        case 'i':
                instruction->instr = INSTR_INC;
                *has_reg = true;
                *has_off = false;
                return input+3;
        case 'j':
                input++;
                switch (*input) {
                case 'm':
                        instruction->instr = INSTR_JMP;
                        *has_reg = false;
                        *has_off = true;
                        return input+2;
                case 'i':
                        input++;
                        switch (*input) {
                        case 'e':
                                instruction->instr = INSTR_JIE;
                                *has_reg = true;
                                *has_off = true;
                                return input+1;
                        case 'o':
                                instruction->instr = INSTR_JIO;
                                *has_reg = true;
                                *has_off = true;
                                return input+1;
                        default:
                                FAIL("Unexpected input");
                        }
                default:
                        FAIL("Unexpected input");
                }
        default:
                FAIL("Unexpected input");
        }
}

static const char *parse_reg(const char *input, struct instruction *instruction) {
        switch (*input) {
        case 'a':
                instruction->reg = REG_A;
                return input+1;
        case 'b':
                instruction->reg = REG_B;
                return input+1;
        default:
                FAIL("Unexpected input");
        }
}

static const char *parse_off(const char *input, struct instruction *instruction) {
        int mult;
        switch (*input) {
        case '+':
                mult = 1; break;
        case '-':
                mult = -1; break;
        default:
                FAIL("Unexpected input");
        }
        input++;

        unsigned n = 0;
        while (*input >= '0' && *input <= '9'){
                n = n * 10 + *input - '0';
                input++;
        }
        instruction->offset = n * mult;

        return input;
}

static const char *parse_line(const char *input, struct instruction *instruction) {
        bool reg, off;
        input = parse_instr(input, instruction, &reg, &off);

        if (reg) {
                ASSERT(*input == ' ', "Unexpected input");
                input++;
                input = parse_reg(input, instruction);
                if (*input == ',') {
                        input++;
                }
        }

        if (off) {
                ASSERT(*input == ' ', "Unexpected input");
                input++;
                input = parse_off(input, instruction);
        }

        return input;
}

static struct instruction *parse_program(const char *input, size_t *len) {
        size_t capacity = 32;
        size_t index = 0;
        struct instruction *instructions = malloc(sizeof(*instructions) * capacity);

        while (*input != '\0') {
                input = parse_line(input, instructions+index);
                ASSERT(*input == '\n', "Unexpected input");
                input++;
                
                index++;
                while (index >= capacity) {
                        capacity *= 2;
                        instructions = realloc(instructions, sizeof(*instructions) * capacity);
                }
        }

        instructions = realloc(instructions, sizeof(*instructions) * index);
        *len = index;
        return instructions;
}

/*
static void run_program(const struct instruction *const program, size_t program_len,
                        unsigned *regA, unsigned *regB) {
        size_t pc = 0;
        while (pc < program_len) {
                const struct instruction *instr = program + pc;
                
                unsigned *reg;
                switch (instr->reg) {
                case REG_A:
                        reg = regA; break;
                case REG_B:
                        reg = regB; break;
                default:
                        FAIL("Illegal register");
                }
                        
                switch(instr->instr) {
                case INSTR_HLF:
                        *reg /= 2;
                        pc++;
                        break;
                case INSTR_TPL:
                        *reg *= 3;
                        pc++;
                        break;
                case INSTR_INC:
                        *reg += 1;
                        pc++;
                        break;
                case INSTR_JMP:
                        pc += instr->offset;
                        break;
                case INSTR_JIE:
                        if (*reg % 2 == 0) {
                                pc += instr->offset; 
                        }
                        break;
                case INSTR_JIO:
                        if (*reg == 1) {
                                pc += instr->offset;
                        }
                        break;
                default:
                        FAIL("Illegal instruction");
                }
        }
}
*/

/*
Analyzing the code, this is what it seems to be doing.
Most likely the value of A is unique for each input, while the algorithm is the same.
Solution: Execute until we have a value of A, then run this algorithm directly in C.
TODO: Haven't I seen this before...? Google it.
DONE: Collatz's Conjecture

a = 9663
b = 0;
while (a != 1) {
  b += 1;
  if (a % 2 == 0) {
    a /= 2;
  } else {
    a *= 3;
    a += 1;
  }
}
return b;

 */

__attribute((const))
static unsigned collatz(unsigned n) {
        unsigned c = 0;
        while (n != 1) {
                c++;
                if (n % 2 == 0) {
                        n /= 2;
                } else {
                        n *= 3;
                        n++;
                }
        }
        return c;
}

static unsigned get_reg_value(const struct instruction *p, unsigned initial) {
        unsigned reg = initial;
        for (; p->instr == INSTR_INC || p->instr == INSTR_TPL; p++) {
                if (p->instr == INSTR_INC) {
                        reg++;
                } else if (p->instr == INSTR_TPL) {
                        reg *= 3;
                }
        }
        return reg;
}

static void solution1(const char *const input, char *const output) {
        size_t program_len;
        struct instruction *program = parse_program(input, &program_len);
        
        unsigned cycles = collatz(get_reg_value(program + 1, 0));
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", cycles);
        free(program);
}

static void solution2(const char *const input, char *const output) {
        size_t program_len;
        struct instruction *program = parse_program(input, &program_len);
        
        unsigned cycles = collatz(get_reg_value(program + program[0].offset, 1));
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", cycles);
        free(program);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
