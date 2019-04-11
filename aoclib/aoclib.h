#ifndef _AOCLIB_H
#define _AOCLIB_H

#include <stddef.h> 

#ifdef DEBUG

#include <stdlib.h>
#include <stdbool.h>

#define FAIL(msg) aoc_fail(msg, __LINE__)
#define ASSERT(cond, msg) aoc_failif(!(cond), msg, __LINE__)

void __attribute__((noreturn)) aoc_fail(const char *const msg, const int srcline);
void aoc_failif(const bool condition, const char *const msg, const int srcline);

#else

#define FAIL(msg) __builtin_unreachable()
#define ASSERT(cond, msg) if (!(cond)) __builtin_unreachable()

#endif

#define OUTPUT_BUFFER_SIZE 256

typedef void (aoc_solution_callback)(const char *const, char *const);
typedef void (aoc_permute_callback)(int *const, void*);
typedef void (aoc_combinations_callback)(int *const, void*);

// Process arguments and call either solution function
// which should take an input and an output it must write to
int aoc_run(const int argc, char *const *const argv, aoc_solution_callback *const solution1, aoc_solution_callback *const solution2);

// Permute the given array of given size and call func with each permutation and with args
void aoc_permute(int *const array, const size_t size, aoc_permute_callback *const func, void *const args);

// Find combinations of n elements in an array of len size.
// When a combination is found call func with two parameters:
//   * pointer to first element of an array of n elements of type int
//   * the args void pointer
// The given array is unmodified. Incorrect parameters will call the FAIL macro.
void aoc_combinations(const int *const array, const size_t len, const size_t n, aoc_combinations_callback *const func, void *const args);

#endif
