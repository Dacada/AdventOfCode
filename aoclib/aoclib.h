#ifndef _AOCLIB_H
#define _AOCLIB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef DEBUG

#define FAIL(msg, ...) aoc_fail(__LINE__, msg, ##__VA_ARGS__)
#define ASSERT(cond, msg, ...) aoc_failif(!(cond), __LINE__, msg, ##__VA_ARGS__)
#define DBG(msg, ...) aoc_dbg(__LINE__, msg, ##__VA_ARGS__)

__attribute__((format(printf, 2, 3))) void __attribute__((noreturn))
aoc_fail(const int srcline, const char *const msg, ...);
__attribute__((format(printf, 3, 4))) void aoc_failif(const bool condition, const int srcline, const char *const msg,
                                                      ...);
__attribute__((format(printf, 2, 3))) void aoc_dbg(const int srcline, const char *const msg, ...);

#else

#define FAIL(msg, ...) __builtin_unreachable()
#define ASSERT(cond, msg, ...)                                                                                         \
  if (!(cond))                                                                                                         \
  __builtin_unreachable()
#define DBG(msg, ...)                                                                                                  \
  do {                                                                                                                 \
  } while (0)

#endif

#define OUTPUT_BUFFER_SIZE 256

#define AOC_ABS(x) ((x) > 0 ? (x) : -(x))
#define AOC_MAX(x, y) ((x) > ((y) ? (x) : (y)))
#define AOC_MIN(x, y) ((x) < ((y) ? (x) : (y)))

// Generic dynamic array implementation
struct aoc_dynarr {
  void *data;
  size_t size;
  int len;
  int cap;
};

// Initialize the array. Size is the size of each element.
void aoc_dynarr_init(struct aoc_dynarr *arr, size_t size, int cap);

void aoc_dynarr_free(struct aoc_dynarr *arr);

// Grow the array by the given amount. Return a pointer to a memory region of
// least amount*size bytes.
void *aoc_dynarr_grow(struct aoc_dynarr *arr, int amount);

// Remove every element from the array but keep memory reserved
void aoc_dynarr_truncate(struct aoc_dynarr *arr);

#define AOC_DYNARR_IDX(arr, idx, type)                                                                                 \
  ({                                                                                                                   \
    struct aoc_dynarr __arr = (arr);                                                                                   \
    int __idx = (idx);                                                                                                 \
    ASSERT(__idx >= 0 && __idx < __arr.len, "aoc_array oob access: %d (len %d)", __idx, __arr.len);                    \
    ASSERT(sizeof(type) == __arr.size, "aoc_array access with invalid type (expected size %d but got %d)", __arr.size, \
           sizeof(type));                                                                                              \
    ASSERT(__arr.data != NULL, "aoc_array initialized access");                                                        \
    type *__ptr = __arr.data;                                                                                          \
    __ptr[__idx];                                                                                                      \
  })

__attribute__((malloc)) __attribute__((alloc_size(1))) __attribute__((returns_nonnull)) void *aoc_malloc(size_t size);
__attribute__((alloc_size(2))) __attribute__((returns_nonnull)) void *aoc_realloc(void *ptr, size_t size);

typedef void(aoc_solution_callback)(const char *const, char *const);
typedef void(aoc_permute_callback)(int *const, void *);
typedef void(aoc_combinations_callback)(int *const, void *);
typedef void(aoc_parse_grid_callback)(const char **, void *, int, int, void *);

// Process arguments and call either solution function
// which should take an input and an output it must write to
int aoc_run(const int argc, char *const *const argv, aoc_solution_callback *const solution1,
            aoc_solution_callback *const solution2);

// Permute the given array of given size and call func with each permutation and
// with args
void aoc_permute(int *const array, const size_t size, aoc_permute_callback *const func, void *const args);

// Find combinations of n elements in an array of len size.
// When a combination is found call func with two parameters:
//   * pointer to first element of an array of n elements of type int
//   * the args void pointer
// The given array is unmodified. Incorrect parameters will call the FAIL macro.
void aoc_combinations(const int *const array, const size_t len, const size_t n, aoc_combinations_callback *const func,
                      void *const args);

// Read the characters in the given "image" and return them as a
// dynamically allocated string. Skip columns of blank characters.
char *aoc_ocr(const char *image, size_t image_width, size_t image_height);

// Parse a grid from input, where rows are separated by newline characters. All
// rows must have the same number of columns. The grid is parsed using a
// callback function that processes each grid cell. The callback takes five
// arguments: a pointer to the current position in the input string
// (modifiable), a pointer to the memory where the parsed cell value should be
// written, the current x coordinate we're parsing, the current y coordinate and
// the args pointer passed to the function (for reentrant passing of additional
// arguments to the callback). The function parses until it encounters either a
// null character or two newlines in a row and leaves the pointer pointing to
// the character where it finished parsing (either the second newline or the
// null character).
//
// Example: Parsing space-separated 2-digit numbers into an integer grid:
//
// static void callback(const char **input, void *parsed, int x, int y, void
// *args) {
//     int n = 0;
//     ASSERT(isdigit(**input), "parse error");
//     n += **input - '0';
//     *input += 1;
//     n *= 10;
//     ASSERT(isdigit(**input), "parse error");
//     n += **input - '0';
//     *input += 1;
//     if (**input == ' ') *input += 1;
//     *((int*)parsed) = n;
// }
//
// char *input = ...
// int width, height;
// int *numbers = aoc_parse_grid(&input, callback, sizeof(int), &height, &width,
// NULL);
//
// Access elements: numbers[row * width + col]
void *aoc_parse_grid(const char **input, aoc_parse_grid_callback callback, size_t size, int *height, int *width,
                     void *args);

// Some generic grid parsers that don't need callbacks
char *aoc_parse_grid_chars(const char **input, int *height, int *width);
int *aoc_parse_grid_digits(const char **input, int *height, int *width);

// Parse a sequence of numbers from the input, stopping at the first non digit
// character, as a base 10 unsigned integer. Fail if the first character pointed
// to by the input is not a digit.
int aoc_parse_int(const char **input);
long aoc_parse_long(const char **input);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=pure"
int aoc_cmp_int(const void *v1, const void *v2);
#pragma GCC diagnostic pop

#endif
