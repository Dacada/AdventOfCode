#ifndef _AOCLIB_H
#define _AOCLIB_H

#define OUTPUT_BUFFER_SIZE 256

typedef void (aoc_solution_callback)(char*, char*);

int aoc_run(int argc, char *argv[], aoc_solution_callback solution1, aoc_solution_callback solution2);

#endif
