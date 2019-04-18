#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>

#define GRIDSIZE 100

static bool lights[2][GRIDSIZE][GRIDSIZE];
static int current = 0;
static int other = 1;

static void parse(const char *const input) {
  size_t i=0;
  for (size_t j=0; j<GRIDSIZE; j++) {
    for (size_t k=0; k<GRIDSIZE; k++) {
      char c = input[i++];
      if (c == '#') {
	lights[current][j][k] = true;
      } else {
	// static array starts as 0, so it's already false
	ASSERT(c == '.', "Expected dot");
      }
    }
    ASSERT(input[i] == '\n', "Expected end of line");
    i++;
  }
  ASSERT(input[i] == '\0', "Expected end of string");
}

static unsigned int neighbors(size_t i, size_t j) {
  unsigned int result = 0;
  if (i < GRIDSIZE-1 && j < GRIDSIZE-1 && lights[current][i+1][j+1]) result++;
  if (i < GRIDSIZE-1 &&                   lights[current][i+1][ j ]) result++;
  if (i < GRIDSIZE-1 &&      j > 0     && lights[current][i+1][j-1]) result++;
  if (                  j < GRIDSIZE-1 && lights[current][ i ][j+1]) result++;
  if (                       j > 0     && lights[current][ i ][j-1]) result++;
  if (     i > 0     && j < GRIDSIZE-1 && lights[current][i-1][j+1]) result++;
  if (     i > 0     &&                   lights[current][i-1][ j ]) result++;
  if (     i > 0     &&      j > 0     && lights[current][i-1][j-1]) result++;
  return result;
}

static void step(void) {
  for (size_t i=0; i<GRIDSIZE; i++) {
    for (size_t j=0; j<GRIDSIZE; j++) {
      unsigned int n = neighbors(i, j);
      if (lights[current][i][j]) {
	if (n == 2 || n == 3) {
	  lights[other][i][j] = true;
	} else {
	  lights[other][i][j] = false;
	}
      } else {
	if (n != 3) {
	  lights[other][i][j] = false;
	} else {
	  lights[other][i][j] = true;
	}
      }
    }
  }
}

static void swap(void) {
  int tmp = current;
  current = other;
  other = tmp;
}

static unsigned int count_lights(void) {
  unsigned int count = 0;
  for (int i=0; i<GRIDSIZE; i++) {
    for (int j=0; j<GRIDSIZE; j++) {
      if (lights[current][i][j]) {
	count++;
      }
    }
  }
  return count;
}

/*
static void print_lights(void) {
  for (size_t i=0; i<GRIDSIZE; i++) {
    for (size_t j=0; j<GRIDSIZE; j++) {
      if (lights[current][i][j]) {
	fprintf(stderr, "#");
      } else {
	fprintf(stderr, ".");
      }
    }
    fprintf(stderr, "\n");
  }
}
*/

static void the_light_inside_has_broken_but_i_still_work(void) {
  lights[current][0][0] = '#';
  lights[current][0][GRIDSIZE-1] = '#';
  lights[current][GRIDSIZE-1][0] = '#';
  lights[current][GRIDSIZE-1][GRIDSIZE-1] = '#';
}

static void solution1(const char *const input, char *const output) {  
  parse(input);

  /*
  fprintf(stderr, "Initial state:\n");
  print_lights();
  */
  
  for (int i=0; i<100; i++) {
    step();
    swap();

    /*
    fprintf(stderr, "After %d step:\n", i+1);
    print_lights();
    */
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count_lights());
}

static void solution2(const char *const input, char *const output) {
  parse(input);
  the_light_inside_has_broken_but_i_still_work();

  for (int i=0; i<100; i++) {
    step();
    swap();
    the_light_inside_has_broken_but_i_still_work();
  }
  
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count_lights());
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
