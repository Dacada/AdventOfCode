#include "aoclib.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG

__attribute__((format(printf, 3, 0))) static void aoc_err_prnt(const int srcline, const char *const type,
                                                               const char *const msg, va_list ap) {
  fprintf(stderr, "(line %d) %s: ", srcline, type);
  vfprintf(stderr, msg, ap);
  fprintf(stderr, "\n");
}

void aoc_fail(const int srcline, const char *const msg, ...) {
  va_list ap;
  va_start(ap, msg);
  aoc_err_prnt(srcline, "ASSERT ERROR", msg, ap);
  va_end(ap);
  abort();
}

void aoc_failif(const bool condition, const int srcline, const char *const msg, ...) {
  if (condition) {
    va_list ap;
    va_start(ap, msg);
    aoc_err_prnt(srcline, "ASSERT ERROR", msg, ap);
    va_end(ap);
    abort();
  }
}

void aoc_dbg(const int srcline, const char *const msg, ...) {
  va_list ap;
  va_start(ap, msg);
  aoc_err_prnt(srcline, "DBG", msg, ap);
  va_end(ap);
}

#endif

struct aoc_point aoc_point_move(struct aoc_point p, enum aoc_direction dir) {
  struct aoc_point q = p;
  switch (dir) {
  case AOC_DIRECTION_NORTH:
    q.y--;
    break;
  case AOC_DIRECTION_EAST:
    q.x++;
    break;
  case AOC_DIRECTION_SOUTH:
    q.y++;
    break;
  case AOC_DIRECTION_WEST:
    q.x--;
    break;
  default:
    FAIL("invalid direction %d", dir);
  }
  return q;
}

void aoc_dynarr_init(struct aoc_dynarr *arr, size_t size, int cap) {
  arr->data = aoc_malloc(size * cap);
  arr->size = size;
  arr->len = 0;
  arr->cap = cap;
}

void aoc_dynarr_free(struct aoc_dynarr *arr) { free(arr->data); }

void *aoc_dynarr_grow(struct aoc_dynarr *arr, int amount) {
  int oldlen = arr->len;
  arr->len += amount;

  if (arr->len > arr->cap) {
    do {
      arr->cap *= 2;
    } while (arr->len > arr->cap);
    arr->data = aoc_realloc(arr->data, arr->cap * arr->size);
  }

  return (char *)arr->data + (arr->size * oldlen);
}

void *aoc_dynarr_shrink(struct aoc_dynarr *arr, int amount) {
  ASSERT(arr->len >= amount, "attempt to shrink array of len %d by %d", arr->len, amount);
  arr->len -= amount;
  return (char *)arr->data + (arr->size * arr->len);
}

void aoc_dynarr_truncate(struct aoc_dynarr *arr) { arr->len = 0; }

void *aoc_malloc(size_t size) {
  void *ptr = malloc(size);
  if (ptr == NULL) {
    perror("malloc");
    FAIL("memory error");
  }
  return ptr;
}

void *aoc_realloc(void *ptr, size_t size) {
  void *newptr = realloc(ptr, size);
  if (newptr == NULL) {
    perror("realloc");
    FAIL("memory error");
  }
  return newptr;
}

static char *read_input() {
  int c;
  size_t p4kB = 4096, i = 0;
  char *ptrString = aoc_malloc((p4kB + 1) * sizeof(*ptrString));

  while ((c = getchar()) != EOF) {
    if (i == p4kB * sizeof(char)) {
      p4kB += 4096;
      ptrString = aoc_realloc(ptrString, (p4kB + 1) * sizeof(*ptrString));
    }
    ptrString[i++] = c;
  }

  ptrString[i] = '\0';
  return ptrString;
}

int aoc_run(const int argc, char *const *const argv, aoc_solution_callback *const solution1,
            aoc_solution_callback *const solution2) {
  char output[OUTPUT_BUFFER_SIZE];
  int retcode = 0;

  if (argc != 2) {
    strcpy(output, "ERROR ARGUMENT NUMBER");
    retcode = 1;
  } else {
    char *input = read_input();
    if (input == NULL) {
      strcpy(output, "ERROR READING INPUT");
      retcode = 2;
    }

    if (argv[1][0] == '1') {
      solution1(input, output);
    } else if (argv[1][0] == '2') {
      solution2(input, output);
    } else {
      strcpy(output, "ERROR ARGUMENT NOT 1 OR 2");
      retcode = 3;
    }
    free(input);
  }

  if (output[0] == '\0') {
    strcpy(output, "ERROR NO OUTPUT");
    retcode = 4;
  }

  fputs(output, stdout);
  return retcode;
}

static void swap(int *const array, const size_t i, const size_t j) {
  const int tmp = array[i];
  array[i] = array[j];
  array[j] = tmp;
}

// Heap's algorithm
// https://en.wikipedia.org/wiki/Heap%27s_algorithm
void aoc_permute(int *const array, const size_t size, aoc_permute_callback *const func, void *const args) {
  if (size == 1) {
    func(array, args);
  } else {
    aoc_permute(array, size - 1, func, args);

    for (size_t i = 0; i < size - 1; i++) {
      if (size % 2 == 0) {
        swap(array, i, size - 1);
      } else {
        swap(array, 0, size - 1);
      }

      aoc_permute(array, size - 1, func, args);
    }
  }
}

static void combinations_recursive(const size_t offset, const int *const array, const size_t len, int *const aux,
                                   const size_t n, aoc_combinations_callback *const fun, void *const args) {
  if (n < 1) {
    FAIL("Attempt to make combinations of less than one element");
  } else if (n > len) {
    FAIL("Attempt to make combinations of more elements than the length of the "
         "collection itself");
  } else if (len < 1) {
    return;
  } else if (n == len) {
    memcpy(aux + offset, array, len * sizeof(int));
    fun(aux, args);
  } else if (n == 1) {
    for (size_t i = 0; i < len; i++) {
      aux[offset] = array[i];
      fun(aux, args);
    }
  } else {
    /*
     A B C D E

     A B C
     A B D
     A B E
     A C D
     A C E
     A D E
     B C D
     B C E
     B D E
     C D E

     Iterate so we get A, B, C then in each iteration recurse using
     the rest of the array as the argument and one less element in
     the combination: combinations((A,B,C,D,E), 3) = A + combinations((B,C,D,E),
     2), B + combinations((C,D,E), 2), C + combinations((D,E), 2)
     */
    for (size_t i = 0; i <= len - n; i++) {
      aux[offset] = array[i];
      combinations_recursive(offset + 1, array + i + 1, len - i - 1, aux, n - 1, fun, args);
    }
  }
}

void aoc_combinations(const int *const array, const size_t len, const size_t n, aoc_combinations_callback *const func,
                      void *const args) {
  int *const aux = aoc_malloc(n * sizeof(int));
  combinations_recursive(0, array, len, aux, n, func, args);
  free(aux);
}

struct aoc_ocr_letter {
  bool init;
  size_t width;
  const char *letter;
};

const struct aoc_ocr_letter alphabet[26] = {
    {
        .init = true,
        .width = 4,
        .letter = " ## "
                  "#  #"
                  "#  #"
                  "####"
                  "#  #"
                  "#  #",
    },
    {
        .init = true,
        .width = 4,
        .letter = "### "
                  "#  #"
                  "### "
                  "#  #"
                  "#  #"
                  "### ",
    },
    {
        .init = true,
        .width = 4,
        .letter = " ## "
                  "#  #"
                  "#   "
                  "#   "
                  "#  #"
                  " ## ",
    },
    {
        .init = false,
    },
    {
        .init = true,
        .width = 4,
        .letter = "####"
                  "#   "
                  "### "
                  "#   "
                  "#   "
                  "####",
    },
    {
        .init = true,
        .width = 4,
        .letter = "####"
                  "#   "
                  "### "
                  "#   "
                  "#   "
                  "#   ",
    },
    {
        .init = true,
        .width = 4,
        .letter = " ## "
                  "#  #"
                  "#   "
                  "# ##"
                  "#  #"
                  " ###",
    },
    {
        .init = false,
    },
    {
        .init = true,
        .width = 3,
        .letter = "###"
                  " # "
                  " # "
                  " # "
                  " # "
                  "###",
    },
    {
        .init = true,
        .width = 4,
        .letter = "  ##"
                  "   #"
                  "   #"
                  "   #"
                  "#  #"
                  " ## ",
    },
    {
        .init = true,
        .width = 4,
        .letter = "#  #"
                  "# # "
                  "##  "
                  "# # "
                  "# # "
                  "#  #",
    },
    {
        .init = true,
        .width = 4,
        .letter = "#   "
                  "#   "
                  "#   "
                  "#   "
                  "#   "
                  "####",
    },
    {
        .init = false,
    },
    {
        .init = false,
    },
    {
        .init = false,
    },
    {
        .init = true,
        .width = 4,
        .letter = "### "
                  "#  #"
                  "#  #"
                  "### "
                  "#   "
                  "#   ",
    },
    {
        .init = false,
    },
    {
        .init = true,
        .width = 4,
        .letter = "### "
                  "#  #"
                  "#  #"
                  "### "
                  "# # "
                  "#  #",
    },
    {
        .init = false,
    },
    {
        .init = false,
    },
    {
        .init = true,
        .width = 4,
        .letter = "#  #"
                  "#  #"
                  "#  #"
                  "#  #"
                  "#  #"
                  " ## ",
    },
    {
        .init = false,
    },
    {
        .init = false,
    },
    {
        .init = false,
    },
    {
        .init = true,
        .width = 5,
        .letter = "#   #"
                  "#   #"
                  " # # "
                  "  #  "
                  "  #  "
                  "  #  ",
    },
    {
        .init = true,
        .width = 4,
        .letter = "####"
                  "   #"
                  "  # "
                  " #  "
                  "#   "
                  "####",
    },
};

// Return whether the image has the given letter in the given offset
// matching exactly
static bool aoc_ocr_letter_match(const char *image, size_t image_height, size_t image_width, size_t image_width_offset,
                                 struct aoc_ocr_letter letter) {
  for (size_t i = 0; i < letter.width; i++) {
    if (i + image_width_offset >= image_width) {
      return false;
    }

    for (size_t j = 0; j < image_height; j++) {
      char c = image[image_width * j + i + image_width_offset];
      char l = letter.letter[letter.width * j + i];

      bool equal = (isblank(c) && isblank(l)) || c == l;
      if (!equal) {
        return false;
      }
    }
  }

  return true;
}

// Return whether the image has a blank (all spaces) column in the
// given width offset
static bool aoc_ocr_blank_column(const char *image, size_t image_height, size_t image_width,
                                 size_t image_width_offset) {
  for (size_t j = 0; j < image_height; j++) {
    char c = image[image_width * j + image_width_offset];
    if (!isblank(c)) {
      return false;
    }
  }
  return true;
}

// Read a single character from image using alphabet, skip blank
// columns first
static char aoc_ocr_letter(const char *image, size_t image_width, size_t image_height, size_t *image_width_offset) {
  // Skip any blank columns
  while (aoc_ocr_blank_column(image, image_height, image_width, *image_width_offset)) {
    *image_width_offset += 1;
  }

  for (size_t i = 0; i < 26; i++) {
    if (!alphabet[i].init) {
      continue;
    }

    if (aoc_ocr_letter_match(image, image_height, image_width, *image_width_offset, alphabet[i])) {
      *image_width_offset += alphabet[i].width;
      return 'A' + i;
    }
  }

  return '\0';
}

char *aoc_ocr(const char *image, size_t image_width, size_t image_height) {
  size_t result_cap = 4;
  char *result = aoc_malloc(sizeof(char) * result_cap);
  size_t result_len = 0;

  size_t image_width_offset = 0;

  for (;;) {
    if (result_len >= result_cap) {
      result_cap *= 10;
      result = aoc_realloc(result, sizeof(char) * result_cap);
    }

    char c = aoc_ocr_letter(image, image_width, image_height, &image_width_offset);
    result[result_len++] = c;
    if (c == '\0') {
      break;
    }
  }

  return result;
}

void *aoc_parse_grid(const char **const input, aoc_parse_grid_callback callback, size_t size, int *height, int *width,
                     void *args) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, size, 32);

  int w = 0;
  int h = 0;

  while (**input != '\0') {
    if (**input == '\n') {
      if ((*input)[1] == '\0' || (*input)[1] == '\n') {
        break;
      }
      if (w == 0) {
        w = arr.len;
      }
      h += 1;
      ASSERT(arr.len % w == 0, "row %d has inconsistent column count", w);
      *input += 1;
      continue;
    }

    int x, y;
    if (w == 0) {
      x = arr.len;
      y = 0;
    } else {
      x = arr.len % w;
      y = arr.len / w;
    }

    void *mem = aoc_dynarr_grow(&arr, 1);
    callback(input, mem, x, y, args);
  }

  h += 1;

  ASSERT(arr.len == w * h, "grid parse error: %d != %d * %d (width * height)", arr.len, w, h);

  *height = h;
  *width = w;
  return arr.data;
}

static void aoc_parse_grid_char_callback(const char **input, void *res, int x, int y, void *args) {
  (void)x;
  (void)y;
  (void)args;

  char *result = res;
  *result = **input;
  *input += 1;
}

static void aoc_parse_grid_digit_callback(const char **input, void *res, int x, int y, void *args) {
  (void)x;
  (void)y;
  (void)args;

  int *result = res;
  ASSERT(isdigit(**input), "expected digit but got: %c", **input);
  *result = **input - '0';
  *input += 1;
}

char *aoc_parse_grid_chars(const char **input, int *nrows, int *ncols) {
  return aoc_parse_grid(input, aoc_parse_grid_char_callback, sizeof(char), nrows, ncols, NULL);
}

int *aoc_parse_grid_digits(const char **input, int *nrows, int *ncols) {
  return aoc_parse_grid(input, aoc_parse_grid_digit_callback, sizeof(int), nrows, ncols, NULL);
}

#define AOC_PARSE_NUM(input, type)                                                                                     \
  ({                                                                                                                   \
    ASSERT(isdigit(**input), "parse error, expected %c to be a digit", **input);                                       \
                                                                                                                       \
    type n = **input - '0';                                                                                            \
    *input += 1;                                                                                                       \
                                                                                                                       \
    while (isdigit(**input)) {                                                                                         \
      n *= 10;                                                                                                         \
      n += **input - '0';                                                                                              \
      *input += 1;                                                                                                     \
    }                                                                                                                  \
                                                                                                                       \
    n;                                                                                                                 \
  })

int aoc_parse_int(const char **input) { return AOC_PARSE_NUM(input, int); }
long aoc_parse_long(const char **input) { return AOC_PARSE_NUM(input, long); }
void aoc_skip_space(const char **input) {
  while (isspace(**input)) {
    *input += 1;
  }
}

void aoc_expect_text(const char **input, const char *text, size_t len) {
  ASSERT(strncmp(*input, text, len) == 0, "parse error: expected text '%s' not found", text);
  *input += len;
}

void aoc_expect_char(const char **input, char expect) {
  ASSERT(**input == expect, "parse error: expected character '%c' but found '%c'", expect, **input);
  *input += 1;
}

int aoc_cmp_int(const void *v1, const void *v2) {
  const int *p1 = v1;
  const int *p2 = v2;
  return *p1 - *p2;
}

#define AOC_MODULO(a, b, type)                                                                                         \
  ({                                                                                                                   \
    type result = a % b;                                                                                               \
    if (result < 0) {                                                                                                  \
      result += (b > 0) ? b : -b;                                                                                      \
    }                                                                                                                  \
    result;                                                                                                            \
  })

int aoc_modulo_int(int a, int b) { return AOC_MODULO(a, b, int); }

long aoc_modulo_long(long a, long b) { return AOC_MODULO(a, b, long); }

void aoc_heap_init(struct aoc_heap *heap, size_t size, int cap, aoc_heap_cmp_callback cb) {
  aoc_dynarr_init(&heap->arr, size, cap);
  heap->cmp = cb;
}

void aoc_heap_free(struct aoc_heap *heap) { aoc_dynarr_free(&heap->arr); }

bool aoc_heap_empty(const struct aoc_heap *heap) { return heap->arr.len == 0; }

static void siftup(void *arr, int idx, size_t size, aoc_heap_cmp_callback cmp) {
  while (idx > 0) {
    int parent_idx = (idx - 1) / 2;
    void *current = (char *)arr + idx * size;
    void *parent = (char *)arr + parent_idx * size;

    if (cmp(current, parent) <= 0) {
      break;
    }

    char tmp[size];
    memcpy(tmp, current, size);
    memcpy(current, parent, size);
    memcpy(parent, tmp, size);

    idx = parent_idx;
  }
}

static void siftdown(void *arr, int len, size_t size, aoc_heap_cmp_callback cmp) {
  int idx = 0;

  for (;;) {
    int left_idx = 2 * idx + 1;
    int right_idx = 2 * idx + 2;
    int largest_idx = idx;

    void *current = (char *)arr + idx * size;
    void *left = (char *)arr + left_idx * size;
    void *right = (char *)arr + right_idx * size;
    void *largest = current;

    if (left_idx < len && cmp(left, largest) > 0) {
      largest_idx = left_idx;
      largest = left;
    }

    if (right_idx < len && cmp(right, largest) > 0) {
      largest_idx = right_idx;
      largest = right;
    }

    if (largest_idx == idx) {
      break;
    }

    char tmp[size];
    memcpy(tmp, current, size);
    memcpy(current, largest, size);
    memcpy(largest, tmp, size);

    idx = largest_idx;
  }
}

void aoc_heap_push(struct aoc_heap *heap, void *element) {
  void *new = aoc_dynarr_grow(&heap->arr, 1);
  memcpy(new, element, heap->arr.size);
  siftup(heap->arr.data, heap->arr.len - 1, heap->arr.size, heap->cmp);
}

void aoc_heap_pop(struct aoc_heap *heap, void *result) {
  ASSERT(heap->arr.len > 0, "pop from empty heap");

  memcpy(result, heap->arr.data, heap->arr.size);
  if (heap->arr.len > 1) {
    memcpy(heap->arr.data, (char *)heap->arr.data + (heap->arr.len - 1) * heap->arr.size, heap->arr.size);
  }
  aoc_dynarr_shrink(&heap->arr, 1);
  if (heap->arr.len > 1) {
    siftdown(heap->arr.data, heap->arr.len, heap->arr.size, heap->cmp);
  }
}
