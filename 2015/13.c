#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define PEOPLE 9
#define MAXNAMELEN 16
#define MATSIZE (PEOPLE * PEOPLE)

char seen_names[PEOPLE][MAXNAMELEN];
static int encode_name(const char *const name) {
  int i;
  for (i = 0; i < PEOPLE; i++) {
    if (seen_names[i][0] == '\0') {
      strcpy(seen_names[i], name);
      return i;
    }

    if (strcmp(seen_names[i], name) == 0)
      return i;
  }

  FAIL("Too many names");
}

static int get_index(const int n1, const int n2) { return n1 * PEOPLE + n2; }

int happinesses[MATSIZE];
static void assign_happiness(const int name1, const int name2, const int happiness) {
  ASSERT(name1 != name2, "Cant assign happiness for the same name twice");
  const int i = get_index(name1, name2);
  happinesses[i] = happiness;
}
static int get_happiness(const int name1, const int name2) {
  ASSERT(name1 != name2, "Cant get happiness for the same name twice");
  const int i = get_index(name1, name2);
  return happinesses[i];
}

static int parse_first_name(const char *const input, const int i, char *const name) {
  int j;
  for (j = 0; j < MAXNAMELEN; j++) {
    const char c = input[i + j];
    if (isspace(c)) {
      break;
    }
    name[j] = c;
  }
  name[j] = '\0';

  ASSERT(strncmp(" would ", input + i + j, 7) == 0, "expected ' would '");
  return i + j + 7;
}

static int parse_happiness_gain(const char *const input, const int i, bool *const gain_happiness) {
  if (strncmp("gain ", input + i, 5) == 0) {
    *gain_happiness = true;
  } else if (strncmp("lose ", input + i, 5) == 0) {
    *gain_happiness = false;
  } else {
    FAIL("expected 'gain ' or 'lose '");
  }

  return i + 5;
}

static int parse_happiness_amount(const char *const input, const int i, int *const amount) {
  int number = 0;
  int j;
  for (j = 0; j < 10; j++) {
    const char c = input[i + j];
    if (isspace(c)) {
      break;
    }
    number = number * 10 + c - 0x30;
  }

  *amount = number;
  ASSERT(strncmp(" happiness units by sitting next to ", input + i + j, 36) == 0,
         "expected ' happiness units by sitting next to '");
  return i + j + 36;
}

static int parse_second_name(const char *const input, const int i, char *const name) {
  int j;
  for (j = 0; j < MAXNAMELEN; j++) {
    const char c = input[i + j];
    if (c == '.') {
      break;
    }
    name[j] = c;
  }
  name[j] = '\0';

  return i + j + 1;
}

static int parse_line(const char *const input, int i) {
  char name[MAXNAMELEN];

  i = parse_first_name(input, i, name);
  const int n1 = encode_name(name);

  bool gain_happiness;
  i = parse_happiness_gain(input, i, &gain_happiness);

  int happiness;
  i = parse_happiness_amount(input, i, &happiness);

  i = parse_second_name(input, i, name);
  const int n2 = encode_name(name);

  assign_happiness(n1, n2, gain_happiness ? happiness : -happiness);
  return i;
}

static void parse(const char *const input) {
  for (int i = 0;; i++) {
    if (input[i] == '\0') {
      return;
    }

    i = parse_line(input, i);
    ASSERT(input[i] == '\n', "Did not parse full line");
  }
}

struct t {
  int result;
  size_t size;
};

static void compute_arrangement_max_happiness(int *const arrangement, void *args) {
  struct t *tuple = args;
  int current_happiness = 0;

  int prev = arrangement[0];
  for (size_t i = 1; i < tuple->size; i++) {
    int curr = arrangement[i];

    current_happiness += get_happiness(prev, curr);
    current_happiness += get_happiness(curr, prev);

    prev = curr;
  }
  current_happiness += get_happiness(arrangement[0], arrangement[tuple->size - 1]);
  current_happiness += get_happiness(arrangement[tuple->size - 1], arrangement[0]);

  if (current_happiness > tuple->result) {
    tuple->result = current_happiness;
  }
}

static int solve8(void) {
  int numbers[] = {0, 1, 2, 3, 4, 5, 6, 7};
  struct t tuple;
  tuple.result = INT_MIN;
  tuple.size = PEOPLE - 1;

  aoc_permute(numbers, PEOPLE - 1, compute_arrangement_max_happiness, &tuple);
  return tuple.result;
}

static int solve9(void) {
  int numbers[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  struct t tuple;
  tuple.result = INT_MIN;
  tuple.size = PEOPLE;

  aoc_permute(numbers, PEOPLE, compute_arrangement_max_happiness, &tuple);
  return tuple.result;
}

static void solution1(const char *const input, char *output) {
  parse(input);
  const int total_happiness = solve8();
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total_happiness);
}

static void solution2(const char *const input, char *output) {
  parse(input);

  int me = encode_name("Me");
  for (int i = 0; i < PEOPLE - 1; i++) {
    assign_happiness(me, i, 0);
    assign_happiness(i, me, 0);
  }

  const int total_happiness = solve9();
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total_happiness);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
