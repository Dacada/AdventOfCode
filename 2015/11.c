#include <aoclib.h>
#include <stdio.h>

#define PWLEN 8

static bool get_next_char(char *password, size_t i) {
  switch (password[i]) {
    
  }
}

static void get_next_password(char *password, size_t len) {
  ASSERT(len > 0, "Reached password limit", 0, 0);
  
  if (get_next_char(password, len-1)) {
      get_next_password(password, len-1);
  }
}

static void solution1(char *input, char *output) {
  char password[PWLEN];
  strcpy(password, input);
  get_next_password(password, PWLEN);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", password);
}

static void solution2(char *input, char *output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
