#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define PLACES 8
#define NAMELEN 16
// 8!/(2!*(8-2)!)
#define NUMDIST 28

char names[PLACES][NAMELEN];
static int encode_name(const char *const name) {
  int i;
  for (i = 0; i < PLACES; i++) {
    if (names[i][0] == '\0') {
      strcpy(names[i], name);
      return i;
    }

    if (strcmp(names[i], name) == 0)
      return i;
  }

  FAIL("Too many names");
}

static int get_index(int n1, int n2) {
  ASSERT(n1 != n2, "Cannot get index if n1 equals n2");
  if (n1 > n2) {
    // swap
    n1 = n1 + n2;
    n2 = n1 - n2;
    n1 = n1 - n2;
  }

  switch (n1) {
  case 0:
    switch (n2) {
    case 1:
      return 0;
    case 2:
      return 1;
    case 3:
      return 2;
    case 4:
      return 3;
    case 5:
      return 4;
    case 6:
      return 5;
    case 7:
      return 6;
    default:
      FAIL("Unexpected get_index input");
    }
  case 1:
    switch (n2) {
    case 2:
      return 7;
    case 3:
      return 8;
    case 4:
      return 9;
    case 5:
      return 10;
    case 6:
      return 11;
    case 7:
      return 12;
    default:
      FAIL("Unexpected get_index input");
    }
  case 2:
    switch (n2) {
    case 3:
      return 13;
    case 4:
      return 14;
    case 5:
      return 15;
    case 6:
      return 16;
    case 7:
      return 17;
    default:
      FAIL("Unexpected get_index input");
    }
  case 3:
    switch (n2) {
    case 4:
      return 18;
    case 5:
      return 19;
    case 6:
      return 20;
    case 7:
      return 21;
    default:
      FAIL("Unexpected get_index input");
    }
  case 4:
    switch (n2) {
    case 5:
      return 22;
    case 6:
      return 23;
    case 7:
      return 24;
    default:
      FAIL("Unexpected get_index input");
    }
  case 5:
    switch (n2) {
    case 6:
      return 25;
    case 7:
      return 26;
    default:
      FAIL("Unexpected get_index input");
    }
  case 6:
    switch (n2) {
    case 7:
      return 27;
    default:
      FAIL("Unexpected get_index input");
    }
  }

  FAIL("Unexpected get_index input");
}

unsigned int distances[NUMDIST];
static void assign_distance(const int name1, const int name2, const unsigned int distance) {
  const int i = get_index(name1, name2);
  distances[i] = distance;
}
static unsigned int get_distance(const int name1, const int name2) {
  const int i = get_index(name1, name2);
  return distances[i];
}

static int parse_first_name(const char *const input, int i, char *const name) {
  for (int j = 0;; j++) {
    const char c = input[i + j];
    if (isspace(c)) {
      i += j;
      name[j] = '\0';
      break;
    }
    name[j] = c;
  }

  ASSERT(input[i] == ' ' && input[i + 1] == 't' && input[i + 2] == 'o' && input[i + 3] == ' ' && !isspace(input[i + 4]),
         "expected ' to ' but got something else");

  return i + 4;
}

static int parse_second_name(const char *const input, int i, char *const name) {
  for (int j = 0;; j++) {
    const char c = input[i + j];
    if (isspace(c)) {
      i += j;
      name[j] = '\0';
      break;
    }
    name[j] = c;
  }

  ASSERT(input[i] == ' ' && input[i + 1] == '=' && input[i + 2] == ' ' && !isspace(input[i + 3]),
         "expected ' = ' but got something else");

  return i + 3;
}

static int parse_distance(const char *const input, int i, unsigned int *const dist) {
  *dist = 0;
  for (int j = 0;; j++) {
    const char c = input[i + j];
    if (isspace(c)) {
      i += j;
      break;
    }
    *dist *= 10;
    *dist += c - 0x30;
  }

  ASSERT(input[i] == '\n' && !isspace(input[i + 1]), "expected end of line but got something else");

  return i;
}

static int parse_line(const char *const input, int i) {
  char name[NAMELEN];
  i = parse_first_name(input, i, name);
  const int n1 = encode_name(name);

  i = parse_second_name(input, i, name);
  const int n2 = encode_name(name);

  unsigned int dist;
  i = parse_distance(input, i, &dist);

  assign_distance(n1, n2, dist);
  return i;
}

static void parse(const char *const input) {
  for (int i = 0;; i++) {
    if (input[i] == '\0')
      return;

    i = parse_line(input, i);
    ASSERT(input[i] == '\n', "Did not parse full line");
  }
}

static void compute_path_distance_min(int *const path, void *args) {
  unsigned int *result = args;
  unsigned int current_path_distance = 0;

  int prev = path[0];
  for (int i = 1; i < PLACES; i++) {
    int curr = path[i];

    unsigned int distance = get_distance(prev, curr);
    current_path_distance += distance;

    if (current_path_distance >= *result) {
      return;
    }

    prev = curr;
  }

  *result = current_path_distance;
}

static void compute_path_distance_max(int *const path, void *args) {
  unsigned int *result = args;
  unsigned int current_path_distance = 0;

  int prev = path[0];
  for (int i = 1; i < PLACES; i++) {
    int curr = path[i];

    unsigned int distance = get_distance(prev, curr);
    current_path_distance += distance;

    prev = curr;
  }

  if (current_path_distance > *result) {
    *result = current_path_distance;
  }
}

static unsigned int solve_min(void) {
  unsigned int result = UINT_MAX;
  int numbers[] = {0, 1, 2, 3, 4, 5, 6, 7};
  aoc_permute(numbers, PLACES, compute_path_distance_min, &result);
  return result;
}

static unsigned int solve_max(void) {
  unsigned int result = 0;
  int numbers[] = {0, 1, 2, 3, 4, 5, 6, 7};
  aoc_permute(numbers, PLACES, compute_path_distance_max, &result);
  return result;
}

static void solution1(const char *const input, char *const output) {
  parse(input);
  unsigned int total_distance = solve_min();
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", total_distance);
}

static void solution2(const char *const input, char *const output) {
  parse(input);
  unsigned int total_distance = solve_max();
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", total_distance);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
