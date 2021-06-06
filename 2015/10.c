#include <aoclib.h>
#include <stdio.h>
#include <string.h>

// ez, just get a fuckton of space
#define NUMLEN (1<<23)

char num_[NUMLEN];
char nextnum_[NUMLEN];

char *num = num_;
char *nextnum = nextnum_;

size_t numlen;

static void look_and_say(void) {
  size_t i = 0;
  size_t j = 0;

  while (i < NUMLEN) {
    size_t count = 1;
    
    char n = num[i++];
    if (n == '\0') {
      numlen = j;
      nextnum[j] = '\0';
      char *tmp = num;
      num = nextnum;
      nextnum = tmp;
      return;
    }
    
    while (num[i] == n) {
      count++;
      
      if (++i >= NUMLEN) {
	FAIL("NUMBER GREW TOO BIG!!!!");
      }
    }
    
    nextnum[j++] = count + 0x30;
    nextnum[j++] = n;
  }

  FAIL("NUMBER GREW TOO BIG!!!!");
}

static void solution1(const char *const input, char *const output) {
  strcpy(num, input);
  num[strlen(num)-1] = '\0';
  for (int i=0; i<40; i++) {
    look_and_say();
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", numlen);
}

static void solution2(const char *const input, char *const output) {
  strcpy(num, input);
  num[strlen(num)-1] = '\0';
  for (int i=0; i<50; i++) {
    look_and_say();
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", numlen);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
