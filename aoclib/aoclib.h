#ifndef _AOCLIB_H
#define _AOCLIB_H

#ifdef DEBUG

#include <stdlib.h>
#include <stdbool.h>

#define FAIL(msg, col, lin) aoc_fail(msg, col, lin, __LINE__)
#define ASSERT(cond, msg, col, lin) aoc_failif(!(cond), msg, col, lin, __LINE__)

void __attribute__((noreturn)) aoc_fail(char *msg, size_t column, size_t linenum, int srcline);
void aoc_failif(bool condition, char *msg, size_t column, size_t linenum, int srcline);

#else

#define FAIL(msg, col, lin) __builtin_unreachable()
#define ASSERT(cond, msg, col, lin) if (!(cond)) __builtin_unreachable()

#endif

#define OUTPUT_BUFFER_SIZE 256

typedef void (aoc_solution_callback)(char*, char*);

int aoc_run(int argc, char *argv[], aoc_solution_callback solution1, aoc_solution_callback solution2);

#endif
