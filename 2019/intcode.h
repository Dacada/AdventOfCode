#ifndef INTCODE_H
#define INTCODE_H

#include <stddef.h>
#include <stdbool.h>

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

void machine_init(struct IntCodeMachine *const machine, const char *const input);

bool machine_recv_output(struct IntCodeMachine *const machine, long *output);

bool machine_send_input(struct IntCodeMachine *const machine, long input);

void machine_run(struct IntCodeMachine *machine);

#endif
