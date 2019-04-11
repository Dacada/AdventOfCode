#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>

#define LIGHTS_X 1000
#define LIGHTS_Y 1000

// static variables are always initialized to 0
static unsigned int lights[LIGHTS_Y][LIGHTS_X];

typedef enum { TOGGLE, TURNON, TURNOFF } action_t;

static size_t parse_coords(const char *const string, const size_t start, int *const x, int *const y) {
  size_t i = start;
  *x = 0;
  for (; string[i] != ','; i++) {
    *x = *x*10 + (string[i] - 0x30);
  }

  i += 1;
  *y = 0;
  for (; !isspace(string[i]); i++) {
    *y = *y*10 + (string[i] - 0x30);
  }

  return i;
}

static void action_foreach_light(const int *const coords1, const int *const coords2, unsigned int(*const action)(const unsigned int)) {
  for (int i=coords1[0]; i<=coords2[0]; i++) {
    for (int j=coords1[1]; j<=coords2[1]; j++) {
      lights[j][i] = action(lights[j][i]);
    }
  }
}

static unsigned int toggle_action(const unsigned int input) {
  if (input == 0) {
    return 1;
  } else {
    return 0;
  }
}

static unsigned int turnon_action(const unsigned int input) {
  (void)input;
  return 1;
}

static unsigned int turnoff_action(const unsigned int input) {
  (void)input;
  return 0;
}

static unsigned int inc2_action(const unsigned int input) {
  return input + 2;
}

static unsigned int inc1_action(const unsigned int input) {
  return input + 1;
}

static unsigned int dec1_action(const unsigned int input) {
  if (input == 0) {
    return 0;
  } else {
    return input - 1;
  }
}

static void toggle_lighting(const int *const coords1, const int *const coords2) {
  action_foreach_light(coords1, coords2, toggle_action);
}

static void turnon_lighting(const int *const coords1, const int *const coords2) {
  action_foreach_light(coords1, coords2, turnon_action);
}

static void turnoff_lighting(const int *const coords1, const int *const coords2) {
  action_foreach_light(coords1, coords2, turnoff_action);
}

static void inc2_lighting(const int *const coords1, const int *const coords2) {
  action_foreach_light(coords1, coords2, inc2_action);
}

static void inc1_lighting(const int *const coords1, const int *const coords2) {
  action_foreach_light(coords1, coords2, inc1_action);
}

static void dec1_lighting(const int *const coords1, const int *const coords2) {
  action_foreach_light(coords1, coords2, dec1_action);
}

static void step_lighting_1(const int *const coords1, const int *const coords2, const action_t action) {
  switch (action) {
  case TOGGLE:
    toggle_lighting(coords1, coords2);
    break;
  case TURNON:
    turnon_lighting(coords1, coords2);
    break;
  case TURNOFF:
    turnoff_lighting(coords1, coords2);
    break;
  }
}

static void step_lighting_2(const int *const coords1, const int *const coords2, const action_t action) {
  switch (action) {
  case TOGGLE:
    inc2_lighting(coords1, coords2);
    break;
  case TURNON:
    inc1_lighting(coords1, coords2);
    break;
  case TURNOFF:
    dec1_lighting(coords1, coords2);
    break;
  }
}

static void solution(const char *const input, void(*const step_lighting)(const int *const, const int *const, const action_t)) {
  for (size_t i = 0; input[i] != '\0'; i++) {
    action_t action;
    
    if (input[i+1] == 'o') { // first word must be "toggle"
      i = i + 7;
      action = TOGGLE;
    } else { // first word must be "turn"
      if (input[i+6] == 'n') { // must be "turn on"
	i = i + 8;
	action = TURNON;
      } else { // must be "turn off"
	i = i + 9;
	action = TURNOFF;
      }
    }

    int coords1[2];
    i = parse_coords(input, i, coords1, coords1+1);
    i += 9; // skipping "through"

    int coords2[2];
    i = parse_coords(input, i, coords2, coords2+1);

    step_lighting(coords1, coords2, action);

    while (input[i] != '\n') {
      i++;
    }		      
  }
}

static void solution1(const char *const input, char *const output) {
  solution(input, step_lighting_1);

  unsigned long result = 0;
  for (int i=0; i<LIGHTS_X; i++) {
    for (int j=0; j<LIGHTS_Y; j++) {
      if (lights[j][i] == 1) {
	result += 1;
      }
    }
  }
  
  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
}

static void solution2(const char *const input, char *const output) {
  solution(input, step_lighting_2);

  unsigned long result = 0;
  for (int i=0; i<LIGHTS_X; i++) {
    for (int j=0; j<LIGHTS_Y; j++) {
      result += lights[j][i];
    }
  }
  
  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
