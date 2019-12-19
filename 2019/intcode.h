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

void machine_clone(struct IntCodeMachine *const dest, const struct IntCodeMachine *const src);

void machine_free(struct IntCodeMachine *const machine);

bool machine_recv_output(struct IntCodeMachine *const machine, long *output);

bool machine_send_input(struct IntCodeMachine *const machine, long input);

void machine_run(struct IntCodeMachine *machine);

#endif
