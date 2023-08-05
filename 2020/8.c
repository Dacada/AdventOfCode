#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

enum instr {
        acc,
        jmp,
        nop,
};

struct opcode {
        enum instr instruction;
        int value;
};

static int parse_signed(const char **input) {
        int sign;
        switch (**input) {
        case '+':
                sign = 1;
                break;
        case '-':
                sign = -1;
                break;
        default:
                FAIL("parse error");
        }
        *input += 1;

        int num = 0;
        while (isdigit(**input)) {
                num = num*10 + **input - '0';
                *input += 1;
        }
        return num * sign;
}
static void parse_line(const char **input, struct opcode *const opcode) {
        switch (**input) {
        case 'a':
                opcode->instruction = acc;
                ASSERT(*((*input)+1) == 'c', "parse error");
                ASSERT(*((*input)+2) == 'c', "parse error");
                break;
        case 'j':
                opcode->instruction = jmp;
                ASSERT(*((*input)+1) == 'm', "parse error");
                ASSERT(*((*input)+2) == 'p', "parse error");
                break;
        case 'n':
                opcode->instruction = nop;
                ASSERT(*((*input)+1) == 'o', "parse error");
                ASSERT(*((*input)+2) == 'p', "parse error");
                break;
        default:
                FAIL("parse error");
        }
        
        *input += 3;
        ASSERT(**input == ' ', "parse error");
        *input += 1;

        opcode->value = parse_signed(input);
}
static struct opcode *parse(const char *input, size_t *const program_size) {
        struct opcode *program = malloc(sizeof(*program)*64);
        size_t size = 0;
        size_t capacity = 64;
        
        for (; *input!=0; input++) {
                if (size >= capacity) {
                        capacity *= 2;
                        program = realloc(program, sizeof(*program)*capacity);
                }
                
                parse_line(&input, program+size);
                size++;
                
                ASSERT(*input == '\n', "parse error");
        }
        *program_size = size;

        return program;
}

struct machine {
        size_t pc;
        int accumulator;
        
        bool *seen;
        size_t seen_size;
};

static void reset_machine(struct machine *const machine) {
        machine->pc = 0;
        machine->accumulator = 0;
        memset(machine->seen, 0, sizeof(*machine->seen)*machine->seen_size);
}

static void init_machine(struct machine *const machine, size_t seen_size) {
        machine->seen = malloc(sizeof(bool)*seen_size);
        machine->seen_size = seen_size;
        reset_machine(machine);
}

static void free_machine(struct machine *const machine) {
        free(machine->seen);
        machine->seen = NULL;
}

static bool run_program(struct machine *const machine, const struct opcode *const program,
                        const size_t program_size) {
        bool normal_termination;
        for (;;) {
                size_t pc = machine->pc;
                if (pc == program_size) {
                        normal_termination = true;
                        break;
                }
                ASSERT(pc < program_size, "invalid program");

                if (machine->seen[pc]) {
                        normal_termination = false;
                        break;
                }
                machine->seen[pc] = true;

                struct opcode op = program[pc];
                switch (op.instruction) {
                case acc:
                        machine->accumulator += op.value;
                        break;
                case jmp:
                        machine->pc += op.value - 1;
                        break;
                case nop:
                        break;
                default:
                        FAIL("invalid instruction");
                }
                machine->pc += 1;
        }
        
        return normal_termination;
}

static bool invert_instruction(struct opcode *const op) {
        switch (op->instruction) {
        case acc:
                return false;
        case jmp:
                op->instruction = nop;
                return true;
        case nop:
                op->instruction = jmp;
                return true;
        default:
                FAIL("invalid instruction");
        }
}

static void solution1(const char *const input, char *const output) {
        size_t program_size;
        struct opcode *program = parse(input, &program_size);

        struct machine machine;
        init_machine(&machine, program_size);
        
        run_program(&machine, program, program_size);
        
        free(program);
        free_machine(&machine);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", machine.accumulator);
}

static void solution2(const char *const input, char *const output) {
        size_t program_size;
        struct opcode *program = parse(input, &program_size);

        struct machine machine;
        init_machine(&machine, program_size);

        for (size_t i=0; i<program_size; i++) {
                struct opcode *op = program+i;
                if (!invert_instruction(op)) {
                        continue;
                }

                if (run_program(&machine, program, program_size)) {
                        break;
                }
                
                reset_machine(&machine);
                invert_instruction(op);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", machine.accumulator);
	free_machine(&machine);
	free(program);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
