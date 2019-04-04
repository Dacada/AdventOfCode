#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef DEBUG

static void assert_hex_char(char c) {
  bool b = true;
  b |= c >= '0' && c <= '9';
  b |= c >= 'A' && c <= 'F';
  b |= c >= 'a' && c <= 'f';

  ASSERT(b, "Expected an hexadecimal character");
}

static void assert_hex(char c1, char c2) {
  assert_hex_char(c1);
  assert_hex_char(c2);
}

#else

static void assert_hex(char c1, char c2) {
  (void)c1;
  (void)c2;
}

#endif

static void solution1(char *input, char *output) {
  unsigned int total_chars = 0;
  unsigned int in_memory_chars = 0;

  bool in_string = false;
  bool escaping = false;
  
  for (int i=0;; i++) {
    char c = input[i];
    if (c == '\0') {
      break;
    }

    if (in_string) {
      total_chars++;
      if (escaping) {
	if (c == 'x') {
	  assert_hex(input[i+1], input[i+2]);
	  i += 2;
	  total_chars += 2;
	} else {
	  ASSERT(c == '"' || c == '\\', "Expected quote or backslash to be escaped");
	}
	in_memory_chars++;
	escaping = false;
      } else {

	if (c == '\\') {
	  escaping = true;
	} else if (c == '"') {
	  in_string = false;
	} else {
	  in_memory_chars++;
	}
      }
    } else {
      ASSERT(!escaping, "In escaping state while inbetween strings");
      if (c == '"') {
	total_chars++;
	in_string = true;
      } else {
	ASSERT(isspace(c), "Expected only whitespace inbetween strings");
      }
    }
    
    //fprintf(stderr, "%c total=%u memory=%u\n", c, total_chars, in_memory_chars);
  }
  
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", total_chars - in_memory_chars);
}

static void solution2(char *input, char *output) {
  unsigned int total_chars = 0;
  unsigned int encoded_chars = 0;

  bool in_string = false;
  bool escaping = false;

  for (int i=0;; i++) {
    char c = input[i];
    if (c == '\0') {
      break;
    }

    if (in_string) {
      total_chars++;
      encoded_chars++;
      if (escaping) {
	if (c == 'x') {
	  assert_hex(input[i+1], input[i+2]);
	  i += 2;
	  total_chars += 2;
	  encoded_chars += 2;
	} else {
	  ASSERT(c == '"' || c == '\\', "Expected quote or backslash to be escaped");
	  encoded_chars++;
	}
	escaping = false;
      } else {
	if (c == '\\') {
	  escaping = true;
	  encoded_chars++;
	} else if (c == '"') {
	  in_string = false;
	  encoded_chars += 2;
	}
      }
    } else {
      ASSERT(!escaping, "In escaping state while inbetween strings");
      if (c == '"') {
	total_chars++;
	encoded_chars += 3;
	in_string = true;
      } else {
	ASSERT(isspace(c), "Expected only whitespace inbetween strings");
      }
    }
    
    //fprintf(stderr, "%c total=%u encoded=%u\n", c, total_chars, encoded_chars);
  }
  
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", encoded_chars - total_chars);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
