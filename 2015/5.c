#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>

static void solution1(const char *const input, char *const output) {
  int result = 0;
  
  for (size_t i = 0; input[i] != '\0'; i++) {
    int vowels = 0;
    bool twice_in_a_row = false;
    bool forbidden_pair = false;

    char last_letter = '\0';
    for (; input[i] != '\n' && !forbidden_pair; i++) {
      char c = input[i];

      if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
	vowels++;

      if (c == last_letter) {
	twice_in_a_row = true;
      }

      forbidden_pair =
	(c == 'b' && last_letter == 'a') ||
	(c == 'd' && last_letter == 'c') ||
	(c == 'q' && last_letter == 'p') ||
	(c == 'y' && last_letter == 'x');
      
      last_letter = c;
    }

    if (!forbidden_pair && vowels >= 3 && twice_in_a_row) {
      result += 1;
    }

    while (input[i] != '\n') {
      i++;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *const input, char *const output) {
  int result = 0;

  for (size_t i = 0; input[i] != '\0'; i++) {
    // [0123456789ABCDEF]
    //  ^ ^
    //  j k->
    // 01 == 23 ?
    //
    // [0123456789ABCDEF]
    //  ^  ^
    //  j  k->
    //  01 == 34 ?
    //
    // ...
    //
    // [0123456789ABCDEF]
    //  ^             ^
    //  j             k->
    //  01 == EF ?
    //
    // [0123456789ABCDEF]
    //   ^ ^
    //   j k->
    //  12 == 34 ?
    //
    // ...
    //
    // [0123456789ABCDEF]
    //              ^ ^
    //              j k->
    //  CD == EF ?
    bool repeated_pair = false;
    bool repeated_between = false;

    size_t j;
    for (j=i; input[j+2] != '\n'; j++) {
      if (!repeated_pair && input[j+3] != '\n') {
	for (size_t k=j+2; input[k+1] != '\n'; k++) {
	  //fprintf(stderr, "%c%c == %c%c\n", input[j], input[j+1], input[k], input[k+1]);
	  if (input[j] == input[k] && input[j+1] == input[k+1]) {
	    repeated_pair = true;
	    break;
	  }
	}
      }
      
      if (!repeated_between) {
	if (input[j] == input[j+2]) {
	  repeated_between = true;
	}
      }
    }
    i=j+2;

    if (repeated_pair && repeated_between) {
      result += 1;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
