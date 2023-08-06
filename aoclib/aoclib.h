#ifndef _AOCLIB_H
#define _AOCLIB_H

#include <stddef.h> 
#include <stdlib.h>
#include <stdbool.h>

#ifdef DEBUG


#define FAIL(msg, ...) aoc_fail(__LINE__, msg, ##__VA_ARGS__)
#define ASSERT(cond, msg, ...) aoc_failif(!(cond), __LINE__, msg, ##__VA_ARGS__)
#define DBG(msg, ...) aoc_dbg(__LINE__, msg, ##__VA_ARGS__)

__attribute__((format (printf, 2, 3)))
void __attribute__((noreturn)) aoc_fail(const int srcline, const char *const msg, ...);
__attribute__((format (printf, 3, 4)))
void aoc_failif(const bool condition, const int srcline, const char *const msg, ...);
__attribute__((format (printf, 2, 3)))
void aoc_dbg(const int srcline, const char *const msg, ...);

#else

#define FAIL(msg, ...) __builtin_unreachable()
#define ASSERT(cond, msg, ...) if (!(cond)) __builtin_unreachable()
#define DBG(msg, ...) do{}while(0)

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

// Read the characters in the given "image" and return them as a
// dynamically allocated string. Skip columns of blank characters.
char *aoc_ocr(const char *image, size_t image_width, size_t image_height);

#endif
