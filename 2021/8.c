#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef bool digit[7];
typedef unsigned configuration[7];

#define IDX(array_, letter_) (array_[(letter_) - 'a'])

struct entry {
  digit patterns[10];
  digit outputs[4];
};

static void parse_digit(const char **const input, digit d) {
  memset(d, 0, sizeof(digit));

  while (isalpha(**input)) {
    ASSERT(**input >= 'a' && **input <= 'g', "parse error");
    IDX(d, **input) = true;
    *input += 1;
  }
}

static void parse_line(const char **const input, struct entry *entry) {
  for (int i = 0; i < 10; i++) {
    parse_digit(input, entry->patterns[i]);
    ASSERT(**input == ' ', "parse error");
    *input += 1;
  }
  ASSERT(**input == '|', "parse error");
  *input += 2;
  for (int i = 0; i < 4; i++) {
    parse_digit(input, entry->outputs[i]);
    ASSERT(**input == ' ' || (i == 3 && (**input == '\n' || **input == '\0')), "parse error");
    while (isspace(**input)) {
      *input += 1;
    }
  }
}

static struct entry *parse_input(const char *input, size_t *len) {
  size_t size = 16;
  *len = 0;
  struct entry *list = malloc(sizeof(*list) * size);

  while (*input != '\0') {
    if (*len >= size) {
      size *= 2;
      list = realloc(list, sizeof(*list) * size);
    }
    parse_line(&input, &list[*len]);
    *len += 1;
  }

  return list;
}

static unsigned count_segments(digit d) {
  unsigned count = 0;
  for (int i = 0; i < 7; i++) {
    count += d[i];
  }
  return count;
}

static void solution1(const char *const input, char *const output) {
  size_t nentries;
  struct entry *entries = parse_input(input, &nentries);

  unsigned count = 0;
  for (size_t i = 0; i < nentries; i++) {
    for (int j = 0; j < 4; j++) {
      unsigned n = count_segments(entries[i].outputs[j]);
      if (n == 2 || n == 3 || n == 4 || n == 7) {
        count++;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
  free(entries);
}

static void deduce(struct entry *entry, configuration config) {
  // determine one, four, seven and eight
  int one, four, seven, eight;
  one = four = seven = eight = -1;
  for (int i = 0; i < 10; i++) {
    unsigned n = count_segments(entry->patterns[i]);
    if (n == 2) {
      one = i;
    } else if (n == 4) {
      four = i;
    } else if (n == 3) {
      seven = i;
    } else if (n == 7) {
      eight = i;
    }
  }
  ASSERT(one >= 0 && four >= 0 && seven >= 0 && eight >= 0, "invalid pattern");

  // fdaebc egc gc bgefc ecbagd becfa fagc afgbec gdcbfae bgdef
  //        7   1                     4           8
  // a=
  // b=
  // c=
  // d=
  // e=
  // f=
  // g=

  // determine config['a']
  IDX(config, 'a') = 7;
  for (int i = 0; i < 7; i++) {
    if (entry->patterns[one][i] != entry->patterns[seven][i]) {
      IDX(config, 'a') = i;
    }
  }
  ASSERT(IDX(config, 'a') < 7, "invalid pattern");

  // fdaebc egc gc bgefc ecbagd becfa fagc afgbec gdcbfae bgdef
  //        7   1                     4           8
  // a=e
  // b=
  // c=
  // d=
  // e=
  // f=
  // g=

  // determine six and config['c']
  int six = -1;
  IDX(config, 'c') = 7;
  for (int i = 0; i < 10; i++) {
    unsigned n = count_segments(entry->patterns[i]);
    if (n == 6) {
      for (int j = 0; j < 7; j++) {
        if (!entry->patterns[i][j]) {
          if (entry->patterns[seven][j]) {
            six = i;
            IDX(config, 'c') = j;
          }
          break;
        }
      }
    }
  }
  ASSERT(six >= 0, "invalid pattern");
  ASSERT(IDX(config, 'c') < 7, "invalid pattern");

  // fdaebc egc gc bgefc ecbagd becfa fagc afgbec gdcbfae bgdef
  // 6      7   1                     4           8
  // a=e
  // b=
  // c=g
  // d=
  // e=
  // f=
  // g=

  // determine config['f']
  IDX(config, 'f') = 7;
  for (unsigned j = 0; j < 7; j++) {
    if (IDX(config, 'c') != j && entry->patterns[one][j]) {
      IDX(config, 'f') = j;
    }
  }
  ASSERT(IDX(config, 'f') < 7, "invalid pattern");

  // fdaebc egc gc bgefc ecbagd becfa fagc afgbec gdcbfae bgdef
  // 6      7   1                     4           8
  // a=e
  // b=
  // c=g
  // d=
  // e=
  // f=c
  // g=

  // determine zero, nine, config['d'] and config['e']
  int zero, nine;
  zero = nine = -1;
  IDX(config, 'd') = 7;
  IDX(config, 'e') = 7;
  for (int i = 0; i < 10; i++) {
    if (i == six) {
      continue;
    }

    unsigned n = count_segments(entry->patterns[i]);
    if (n == 6) {
      for (int j = 0; j < 7; j++) {
        if (!entry->patterns[i][j]) {
          if (entry->patterns[four][j]) {
            IDX(config, 'd') = j;
            zero = i;
          } else {
            IDX(config, 'e') = j;
            nine = i;
          }
        }
      }
    }
  }
  ASSERT(zero >= 0, "invalid pattern");
  ASSERT(nine >= 0, "invalid pattern");
  ASSERT(IDX(config, 'd') < 7, "invalid pattern");
  ASSERT(IDX(config, 'e') < 7, "invalid pattern");

  // fdaebc egc gc bgefc ecbagd becfa fagc afgbec gdcbfae bgdef
  // 6      7   1        0            4    9      8
  // a=e
  // b=
  // c=g
  // d=f
  // e=d
  // f=c
  // g=

  // determine config['b']
  IDX(config, 'b') = 7;
  for (unsigned i = 0; i < 7; i++) {
    if (i != IDX(config, 'c') && i != IDX(config, 'd') && i != IDX(config, 'f') && entry->patterns[four][i]) {
      IDX(config, 'b') = i;
      break;
    }
  }
  ASSERT(IDX(config, 'b') < 7, "invalid pattern");

  // fdaebc egc gc bgefc ecbagd becfa fagc afgbec gdcbfae bgdef
  // 6      7   1        0            4    9      8
  // a=e
  // b=a
  // c=g
  // d=f
  // e=d
  // f=c
  // g=

  // determine config['g']
  IDX(config, 'g') = 7;
  for (unsigned i = 0; i < 7; i++) {
    if (i != IDX(config, 'a') && i != IDX(config, 'b') && i != IDX(config, 'c') && i != IDX(config, 'd') &&
        i != IDX(config, 'e') && i != IDX(config, 'f')) {
      IDX(config, 'g') = i;
      break;
    }
  }
  ASSERT(IDX(config, 'g') < 7, "invalid pattern");

  // fdaebc egc gc bgefc ecbagd becfa fagc afgbec gdcbfae bgdef
  // 6      7   1        0            4    9      8
  // a=e
  // b=a
  // c=g
  // d=f
  // e=d
  // f=c
  // g=b

  for (int i = 0; i < 7; i++) {
    for (int j = i + 1; j < 7; j++) {
      ASSERT(config[i] != config[j], "invalid pattern");
    }
  }
}

static unsigned decode(digit d, configuration config) {
  bool correct_digit[7];
  for (int i = 0; i < 7; i++) {
    correct_digit[i] = d[config[i]];
  }

  if (correct_digit['e' - 'a']) {       // e is active, must be 0,2,6,8
    if (correct_digit['d' - 'a']) {     // d is active, must be 2,6,8
      if (correct_digit['f' - 'a']) {   // f is active, must be 6,8
        if (correct_digit['c' - 'a']) { // c is active, must be 8
          ASSERT(correct_digit[0] && correct_digit[1] && correct_digit[2] && correct_digit[3] && correct_digit[4] &&
                     correct_digit[5] && correct_digit[6],
                 "cannot identify number");
          return 8;
        } else { // c is innactive, must be 6
          ASSERT(correct_digit[0] && correct_digit[1] && !correct_digit[2] && correct_digit[3] && correct_digit[4] &&
                     correct_digit[5] && correct_digit[6],
                 "cannot identify number");
          return 6;
        }
      } else { // f is innactive, must be 2
        ASSERT(correct_digit[0] && !correct_digit[1] && correct_digit[2] && correct_digit[3] && correct_digit[4] &&
                   !correct_digit[5] && correct_digit[6],
               "cannot identify number");
        return 2;
      }
    } else { // d is innactive, must be 0
      ASSERT(correct_digit[0] && correct_digit[1] && correct_digit[2] && !correct_digit[3] && correct_digit[4] &&
                 correct_digit[5] && correct_digit[6],
             "cannot identify number");
      return 0;
    }
  } else {                              // e is innactive, must be 1,3,4,5,7,9
    if (correct_digit['b' - 'a']) {     // b is active, must be 4,5,9
      if (correct_digit['a' - 'a']) {   // a is active, must be 5,9
        if (correct_digit['c' - 'a']) { // c is active, must be 9
          ASSERT(correct_digit[0] && correct_digit[1] && correct_digit[2] && correct_digit[3] && !correct_digit[4] &&
                     correct_digit[5] && correct_digit[6],
                 "cannot identify number");
          return 9;
        } else { // c is innactive, must be 5
          ASSERT(correct_digit[0] && correct_digit[1] && !correct_digit[2] && correct_digit[3] && !correct_digit[4] &&
                     correct_digit[5] && correct_digit[6],
                 "cannot identify number");
          return 5;
        }
      } else { // a is innactive, must be 4
        ASSERT(!correct_digit[0] && correct_digit[1] && correct_digit[2] && correct_digit[3] && !correct_digit[4] &&
                   correct_digit[5] && !correct_digit[6],
               "cannot identify number");
        return 4;
      }
    } else {                            // b is innactive, must be 1,3,7
      if (correct_digit['a' - 'a']) {   // a is active, must be 3,7
        if (correct_digit['d' - 'a']) { // d is active, must be 3
          ASSERT(correct_digit[0] && !correct_digit[1] && correct_digit[2] && correct_digit[3] && !correct_digit[4] &&
                     correct_digit[5] && correct_digit[6],
                 "cannot identify number");
          return 3;
        } else { // d is innactive, must be 7
          ASSERT(correct_digit[0] && !correct_digit[1] && correct_digit[2] && !correct_digit[3] && !correct_digit[4] &&
                     correct_digit[5] && !correct_digit[6],
                 "cannot identify number");
          return 7;
        }
      } else { // a is innactive, must be 1
        ASSERT(!correct_digit[0] && !correct_digit[1] && correct_digit[2] && !correct_digit[3] && !correct_digit[4] &&
                   correct_digit[5] && !correct_digit[6],
               "cannot identify number");
        return 1;
      }
    }
  }
}

static void solution2(const char *const input, char *const output) {
  size_t nentries;
  struct entry *entries = parse_input(input, &nentries);

  unsigned result = 0;
  for (size_t i = 0; i < nentries; i++) {
    configuration config;
    deduce(&entries[i], config);

    unsigned number = 0;
    for (int j = 0; j < 4; j++) {
      number *= 10;
      number += decode(entries[i].outputs[j], config);
    }
    result += number;
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
  free(entries);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
