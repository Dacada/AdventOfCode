#include "aoclib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG

void aoc_fail(char *msg, int srcline) {
  fprintf(stderr, "(line %d) ASSERT ERROR: %s\n", srcline, msg);
  abort();
}

void aoc_failif(bool condition, char *msg, int srcline) {
  if (condition) {
    aoc_fail(msg, srcline);
  }
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

int aoc_run(int argc, char *argv[], aoc_solution_callback solution1, aoc_solution_callback solution2) {
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

static void swap(int *array, size_t i, size_t j) {
  int tmp = array[i];
  array[i] = array[j];
  array[j] = tmp;
}

// Heap's algorithm
// https://en.wikipedia.org/wiki/Heap%27s_algorithm
void aoc_permute(int *array, size_t size, aoc_permute_callback func, void *args) {
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
