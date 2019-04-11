#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>

#define CHILDREN 3
#define CATS 7
#define SAMOYEDS 2
#define POMERANIANS 3
#define AKITAS 0
#define VIZSLAS 0
#define GOLDFISH 5
#define TREES 3
#define CARS 2
#define PERFUMES 1

#define ISNUM(n) ((n) >= '0' && (n) <= '9')

static bool(*akitas_ok)(const unsigned int n);
static bool(*cars_ok)(const unsigned int n);
static bool(*cats_ok)(const unsigned int n);
static bool(*children_ok)(const unsigned int n);
static bool(*goldfish_ok)(const unsigned int n);
static bool(*perfumes_ok)(const unsigned int n);
static bool(*pomeranians_ok)(const unsigned int n);
static bool(*samoyeds_ok)(const unsigned int n);
static bool(*trees_ok)(const unsigned int n);
static bool(*vizslas_ok)(const unsigned int n);

static bool akitas_ok1(const unsigned int n)      { return n == AKITAS; }
static bool cars_ok1(const unsigned int n)        { return n == CARS; }
static bool cats_ok1(const unsigned int n)        { return n == CATS; }
static bool children_ok1(const unsigned int n)    { return n == CHILDREN; }
static bool goldfish_ok1(const unsigned int n)    { return n == GOLDFISH; }
static bool perfumes_ok1(const unsigned int n)    { return n == PERFUMES; }
static bool pomeranians_ok1(const unsigned int n) { return n == POMERANIANS; }
static bool samoyeds_ok1(const unsigned int n)    { return n == SAMOYEDS; }
static bool trees_ok1(const unsigned int n)       { return n == TREES; }
static bool vizslas_ok1(const unsigned int n)     { return n == VIZSLAS; }

static bool akitas_ok2(const unsigned int n)      { return n == AKITAS; }
static bool cars_ok2(const unsigned int n)        { return n == CARS; }
static bool cats_ok2(const unsigned int n)        { return n > CATS; }
static bool children_ok2(const unsigned int n)    { return n == CHILDREN; }
static bool goldfish_ok2(const unsigned int n)    { return n < GOLDFISH; }
static bool perfumes_ok2(const unsigned int n)    { return n == PERFUMES; }
static bool pomeranians_ok2(const unsigned int n) { return n < POMERANIANS; }
static bool samoyeds_ok2(const unsigned int n)    { return n == SAMOYEDS; }
static bool trees_ok2(const unsigned int n)       { return n > TREES; }
static bool vizslas_ok2(const unsigned int n)     { return n == VIZSLAS; }

static unsigned int parse_number(const char *const input, unsigned int *const i) {
  unsigned int j = *i;
  while (!ISNUM(input[j])) {
    j++;
  }
  unsigned int num = 0;
  while (ISNUM(input[j])) {
    num = num*10 + input[j]-0x30;
    j++;
  }
  *i = j;
  return num;
}

static bool parse_pair(const char *const input, unsigned int *i) {
  unsigned int j = *i;
  while (input[j] < 'a' || input[j] > 'z') {
    j++;
  }

  bool ret;
  if (input[j] == 'a') { // akitas
    j += 6;
    ret = akitas_ok(parse_number(input, &j));
  } else if (input[j] == 'c') { // cars, cats or children
    j += 2;
    if (input[j] == 'r') { // cars
      j += 2;
      ret = cars_ok(parse_number(input, &j));
    } else if (input[j] == 't') { // cats
      j += 2;
      ret = cats_ok(parse_number(input, &j));
    } else if (input[j] == 'i') { // children
      j += 6;
      ret = children_ok(parse_number(input, &j));
    } else {
      FAIL("Unexpected key");
    }
  } else if (input[j] == 'g') { // goldfish
    j += 8;
    ret = goldfish_ok(parse_number(input, &j));
  } else if (input[j] == 'p') { // perfumes, pomeranians
    j += 1;
    if (input[j] == 'e') { // perfumes
      j += 7;
      ret = perfumes_ok(parse_number(input, &j));
    } else if (input[j] == 'o') { // pomeranians
      j += 10;
      ret = pomeranians_ok(parse_number(input, &j));
    } else {
      FAIL("Unexpected key");
    }
  } else if (input[j] == 's') { // samoyeds
    j += 8;
    ret = samoyeds_ok(parse_number(input, &j));
  } else if (input[j] == 't') { // trees
    j += 5;
    ret = trees_ok(parse_number(input, &j));
  } else if (input[j] == 'v') { // vizslas
    j += 7;
    ret = vizslas_ok(parse_number(input, &j));
  } else {
    FAIL("Unexpected key");
  }

  *i = j;
  return ret;
}

static void skip_to_eol(const char *const input, unsigned int *const i) {
  int j = *i;
  while (input[j] != '\n') {
    j++;
  }
  *i = j;
}

static unsigned int parse_line(const char *const input, unsigned int *const i) {
  unsigned int aunt = parse_number(input, i);
  
  if (!parse_pair(input, i)) {
    skip_to_eol(input, i);
    return 0;
  }
  
  if (!parse_pair(input, i)) {
    skip_to_eol(input, i);
    return 0;
  }

  if (!parse_pair(input, i)) {
    return 0;
  }

  return aunt;
}

static unsigned int parse(const char *const input) {
  for (unsigned int i=0;; i++) {
    if (input[i] == '\0') {
      break;
    }

    unsigned int aunt = parse_line(input, &i);
    if (aunt != 0) {
      return aunt;
    }
    ASSERT(input[i] == '\n', "Did not parse full line");
  }

  FAIL("Did not find Aunt Sue");
}

static void solution1(const char *const input, char *const output) {
  akitas_ok = akitas_ok1;
  cars_ok = cars_ok1;
  cats_ok = cats_ok1;
  children_ok = children_ok1;
  goldfish_ok = goldfish_ok1;
  perfumes_ok = perfumes_ok1;
  pomeranians_ok = pomeranians_ok1;
  samoyeds_ok = samoyeds_ok1;
  trees_ok = trees_ok1;
  vizslas_ok = vizslas_ok1;
  
  unsigned int aunt = parse(input);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", aunt);
}

static void solution2(const char *const input, char *const output) {
  akitas_ok = akitas_ok2;
  cars_ok = cars_ok2;
  cats_ok = cats_ok2;
  children_ok = children_ok2;
  goldfish_ok = goldfish_ok2;
  perfumes_ok = perfumes_ok2;
  pomeranians_ok = pomeranians_ok2;
  samoyeds_ok = samoyeds_ok2;
  trees_ok = trees_ok2;
  vizslas_ok = vizslas_ok2;
  
  unsigned int aunt = parse(input);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", aunt);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
