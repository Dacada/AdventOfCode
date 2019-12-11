#include "aoclib.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG

static void aoc_err_prnt(const int srcline, const char *const type, const char *const msg, va_list ap) {
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

// from
// https://stackoverflow.com/questions/27097915/read-all-data-from-stdin-c
static char *read_input() {
  int c;
  size_t p4kB = 4096, i = 0;
  void *newPtr = NULL;
  char *ptrString = malloc(p4kB * sizeof(char));

  while (ptrString != NULL && (c = getchar()) != EOF) {
    if (i == p4kB * sizeof(char)) {
      p4kB += 4096;
      if ((newPtr = realloc(ptrString, p4kB * sizeof (char))) != NULL) {
	ptrString = (char*) newPtr;
      } else {
	free(ptrString);
	return NULL;
      }
    }
    ptrString[i++] = c;
  }

  if (ptrString != NULL) {
    ptrString[i] = '\0';
    ptrString = realloc(ptrString, strlen(ptrString) + 1);
  } else {
    return NULL;
  }

  return ptrString;
}

int aoc_run(const int argc, char *const *const argv, aoc_solution_callback *const solution1, aoc_solution_callback *const solution2) {
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

    for (size_t i=0; i < size-1; i++) {
      if (size % 2 == 0) {
	swap(array, i, size-1);
      } else {
	swap(array, 0, size-1);
      }

      aoc_permute(array, size - 1, func, args);
    }
  }
}

static void combinations_recursive(const size_t offset, const int *const array, const size_t len, int *const aux, const size_t n, aoc_combinations_callback *const fun, void *const args) {
  if (n < 1) {
    FAIL("Attempt to make combinations of less than one element");
  } else if (n > len) {
    FAIL("Attempt to make combinations of more elements than the length of the collection itself");
  } else if (len < 1) {
    return;
  } else if (n == len) {
    memcpy(aux+offset, array, len*sizeof(int));
    fun(aux, args);
  } else if (n == 1) {
    for (size_t i=0; i<len; i++) {
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
     the combination: combinations((A,B,C,D,E), 3) = A + combinations((B,C,D,E), 2), B + combinations((C,D,E), 2), C + combinations((D,E), 2)
     */
    for (size_t i=0; i<=len-n; i++) {
      aux[offset] = array[i];
      combinations_recursive(offset+1, array+i+1, len-i-1, aux, n-1, fun, args);
    }
  }
}

void aoc_combinations(const int *const array, const size_t len, const size_t n, aoc_combinations_callback *const func, void *const args) {
  int *const aux = malloc(n * sizeof(int));
  combinations_recursive(0, array, len, aux, n, func, args);
  free(aux);
}
