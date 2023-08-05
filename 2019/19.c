#include "intcode.h"
#include <aoclib.h>
#include <stdio.h>

long *ptr = NULL;
static bool drone(struct IntCodeMachine *original, long x, long y) {
        struct IntCodeMachine machine;
        long out;

        machine_clone_static(&machine, original, ptr);
                        
        machine_run(&machine);
        ASSERT(machine_send_input(&machine, x), "Can't input?");
        machine_run(&machine);
        ASSERT(machine_send_input(&machine, y), "Can't input?");
        machine_run(&machine);
        ASSERT(machine_recv_output(&machine, &out), "Can't output?");
        
        return out;
}

static void solution1(const char *const input, char *const output) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);
        ptr = malloc(sizeof(*ptr) * machine.program_size);

        int count = 0;
        for (int i=0; i<50; i++) {
                for (int j=0; j<50; j++) {
                        if (drone(&machine, i, j)) {
                                count++;
#ifdef DEBUG
                                fprintf(stderr, "#");
#endif
                        } else {
#ifdef DEBUG
                                fprintf(stderr, ".");
#endif
                        }
                }
                
#ifdef DEBUG
                fprintf(stderr, "\n");
#endif
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
	machine_free(&machine);
}

static int fitsx(struct IntCodeMachine *machine, long *x, long y) {
        int count = 0;
        
        DBG("Starting at x=%ld", *x);

        if (!drone(machine, *x+100, y)) {
                DBG("Got %d", count);
                return count;
        }

        long offset = 0;
        for (offset=0; offset<10; offset++) {
                if (drone(machine, *x+offset, y)) {
                        count++;
                        break;
                }
        }
        *x += offset;
        
        if (!drone(machine, *x+100, y)) {
                DBG("Got %d", count);
                return count;
        }

        count += 100;

        long base = *x+100;
        long lower_bound = 0;
        long upper_bound = 1<<6;
        while (drone(machine, base+upper_bound, y)) {
                lower_bound = upper_bound;
                upper_bound <<= 1;
        }

        do {
                long middle = (lower_bound + upper_bound)/2;
                if (drone(machine, base+middle, y)) {
                        lower_bound = middle;
                } else {
                        upper_bound = middle;
                }
        } while (upper_bound - lower_bound > 1);

        if (lower_bound == upper_bound || drone(machine, base+lower_bound, y)) {
                count += lower_bound;
        } else {
                count += upper_bound;
        }
        
        return count;
}

static void solution2(const char *const input, char *const output) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);
        ptr = malloc(sizeof(*ptr) * machine.program_size);
        
        long besti = 0;
        for (long j=0;; j++) {
                int extrax = fitsx(&machine, &besti, j) - 100;

                if (!drone(&machine, besti+extrax, j+99)) {
                        continue;
                }

                int lower_bound = 0;
                int upper_bound = extrax;
                do {
                        int middle = (lower_bound + upper_bound) / 2;
                        if (drone(&machine, besti+middle, j+99)) {
                                upper_bound = middle;
                        } else {
                                lower_bound = middle;
                        }
                } while (upper_bound - lower_bound > 1);

                long result;
                if (lower_bound == upper_bound || drone(&machine, besti+lower_bound, j+99)) {
                        result = (besti+lower_bound)*10000+j;
                } else {
                        result = (besti+upper_bound)*10000+j;
                }
                
                snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
		machine_free(&machine);
                return;
        }
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
