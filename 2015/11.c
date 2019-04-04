#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define PWLEN 8

static bool has_increasing_straight_of_three(char *password) {
  char last = password[0];
  int found = 1;
  
  for (int i=1; i<PWLEN; i++) {
    char c = password[i];
    if (c == last+1) {
      found++;
      if (found == 3) {
	return true;
      }
    } else {
      found = 1;
    }
    last = c;
  }

  return false;
}

static bool has_two_pairs(char *password) {
  char last = password[0];
  int pairs_found = 0;
  bool just_found_pair = false;

  for (int i=1; i<PWLEN; i++) {
    char c = password[i];
    if (c == last && !just_found_pair) {
      pairs_found++;
      if (pairs_found == 2) {
	return true;
      }
      just_found_pair = true;
    } else {
      just_found_pair = false;
    }
    last = c;
  }

  return false;
}

static bool is_valid_password(char *password) {
  return has_increasing_straight_of_three(password) &&\
    has_two_pairs(password);
}

static bool get_next_char(char *password, size_t i) {
  switch (password[i]) {
  case 'h':
  case 'n':
  case 'k':
    password[i] += 2;
    return false;
  case 'z':
    password[i] = 'a';
    return true;
  default:
    password[i]++;
    return false;
  }
}

static void get_next_valid_password_recurse(char *password, size_t len) {
  ASSERT(len > 0, "Reached password limit");

  if (get_next_char(password, len-1)) {
    get_next_valid_password_recurse(password, len-1);
  }
}

static void get_next_valid_password(char *password) {
  do {
    get_next_valid_password_recurse(password, PWLEN);
  } while (!is_valid_password(password));
}

static void solution1(char *input, char *output) {
  char password[PWLEN];
  strcpy(password, input);
  get_next_valid_password(password);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", password);
}

static void solution2(char *input, char *output) {
  char password[PWLEN];
  strcpy(password, input);
  get_next_valid_password(password);
  get_next_valid_password(password);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", password);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
