#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define FLOORS 4
#define FLOORS_BITS 2
#define MAX_ROCKS 7
#define QUEUE_SIZE 1 << 19
#define POSSIBLE_STATES 1 << ((2 * MAX_ROCKS + 1) * FLOORS_BITS)

unsigned total_rocks;

const char *floor_names[FLOORS] = {
    "first",
    "second",
    "third",
    "fourth",
};

struct state {
  unsigned steps;
  unsigned elevator_floor;
  unsigned rock_floor[MAX_ROCKS];
  unsigned chip_floor[MAX_ROCKS];
};

static size_t state_packed(const struct state *const state) {
  size_t result = 0;
  result |= state->elevator_floor;
  result <<= FLOORS_BITS;
  for (unsigned i = 0; i < total_rocks; i++) {
    result |= state->rock_floor[i];
    result <<= FLOORS_BITS;
    result |= state->chip_floor[i];
    result <<= FLOORS_BITS;
  }
  result >>= FLOORS_BITS;
  return result;
}

// the equivalent states of state
// rock_floor[0] = r_0 | chip_floor[0] = c_0
// rock_floor[1] = r_1 | chip_floor[1] = c_1
// rock_floor[2] = r_2 | chip_floor[2] = c_2
// are
// rock_floor[0] = r_1 | chip_floor[0] = c_1
// rock_floor[1] = r_0 | chip_floor[1] = c_0
// rock_floor[2] = r_2 | chip_floor[2] = c_2
// ,
// rock_floor[0] = r_2 | chip_floor[0] = c_2
// rock_floor[1] = r_0 | chip_floor[1] = c_0
// rock_floor[2] = r_1 | chip_floor[2] = c_1
// ,
// rock_floor[0] = r_0 | chip_floor[0] = c_0
// rock_floor[1] = r_2 | chip_floor[1] = c_2
// rock_floor[2] = r_1 | chip_floor[2] = c_1
// ,
// rock_floor[0] = r_1 | chip_floor[0] = c_1
// rock_floor[1] = r_2 | chip_floor[1] = c_2
// rock_floor[2] = r_0 | chip_floor[2] = c_0
// and
// rock_floor[0] = r_2 | chip_floor[0] = c_2
// rock_floor[1] = r_1 | chip_floor[1] = c_1
// rock_floor[2] = r_0 | chip_floor[2] = c_0

struct permute_args {
  const struct state *state;
  bool *seen_states;
};

static void set_true_for_permutation(int *const permutation, void *argsv) {
  struct permute_args *args = argsv;

  struct state permuted_state = *args->state;
  for (unsigned i = 0; i < total_rocks; i++) {
    permuted_state.rock_floor[i] = args->state->rock_floor[permutation[i]];
    permuted_state.chip_floor[i] = args->state->chip_floor[permutation[i]];
  }

  size_t packed = state_packed(&permuted_state);
  args->seen_states[packed] = true;
}

static void set_true_for_packed_equivalent_states(const struct state *const state, bool *const seen_states) {
  int indices[total_rocks];
  for (int i = 0; i < (int)total_rocks; i++) {
    indices[i] = i;
  }

  struct permute_args args;
  args.state = state;
  args.seen_states = seen_states;

  aoc_permute(indices, total_rocks, set_true_for_permutation, &args);
}

#ifdef DEBUG
static void print_state(const struct state *const state) {
  fprintf(stderr, "[steps=%u]\n", state->steps);
  for (int f = FLOORS - 1; f >= 0; f--) {
    unsigned floor = (unsigned)f;

    char elevator;
    if (state->elevator_floor == floor) {
      elevator = 'E';
    } else {
      elevator = '.';
    }

    char rocks_and_chips[total_rocks * 2 * 3 + 1];
    for (unsigned rock = 0; rock < total_rocks; rock++) {
      unsigned i = rock * 2 * 3;
      if (state->rock_floor[rock] == floor) {
        rocks_and_chips[i + 0] = rock + '0';
        rocks_and_chips[i + 1] = 'G';
      } else {
        rocks_and_chips[i + 0] = '.';
        rocks_and_chips[i + 1] = ' ';
      }
      rocks_and_chips[i + 2] = ' ';

      if (state->chip_floor[rock] == floor) {
        rocks_and_chips[i + 3] = rock + '0';
        rocks_and_chips[i + 4] = 'M';
      } else {
        rocks_and_chips[i + 3] = '.';
        rocks_and_chips[i + 4] = ' ';
      }
      rocks_and_chips[i + 5] = ' ';
    }
    rocks_and_chips[total_rocks * 2 * 3] = '\0';

    fprintf(stderr, "F%u %c  %s\n", floor + 1, elevator, rocks_and_chips);
  }
  fprintf(stderr, "\n");
}
#else
static void print_state(const struct state *const state) { (void)state; }
#endif

static unsigned get_rock_idx_from_name(const char *const rock_name, char *rock_names[MAX_ROCKS]) {
  for (unsigned i = 0; i < MAX_ROCKS; i++) {
    if (rock_names[i] == NULL) {
      rock_names[i] = strdup(rock_name);
      return i;
    } else if (strcmp(rock_names[i], rock_name) == 0) {
      return i;
    }
  }

  FAIL("parse error");
}

static char *read_word(const char **const input) {
  size_t len = 0;
  size_t cap = 16;
  char *word = malloc(sizeof(*word) * cap);
  while (isalnum(**input)) {
    if (len >= cap) {
      cap *= 2;
      word = realloc(word, sizeof(*word) * cap);
    }
    word[len++] = **input;
    *input += 1;
  }
  if (len >= cap) {
    word = realloc(word, sizeof(*word) * (cap + 1));
  }
  word[len] = '\0';
  return word;
}

static bool can_skip_text(const char *const input, const char *const text) {
  return strncmp(input, text, strlen(text)) == 0;
}

static void skip_text(const char **const input, const char *const text) {
  ASSERT(can_skip_text(*input, text), "parse error");
  *input += strlen(text);
}

static bool try_skip_text(const char **const input, const char *const text) {
  if (can_skip_text(*input, text)) {
    *input += strlen(text);
    return true;
  }
  return false;
}

static bool parse_element(const char **const input, unsigned rock_floor[MAX_ROCKS], unsigned chip_floor[MAX_ROCKS],
                          unsigned floor, char *rock_names[MAX_ROCKS]) {
  skip_text(input, "a ");
  char *rock_name = read_word(input);
  unsigned rock = get_rock_idx_from_name(rock_name, rock_names);
  free(rock_name);

  switch (**input) {
  case '-':
    chip_floor[rock] = floor;
    skip_text(input, "-compatible microchip");
    break;
  case ' ':
    rock_floor[rock] = floor;
    skip_text(input, " generator");
    break;
  default:
    FAIL("parse error");
  }

  if (**input == '.') {
    return false;
  }

  if (!try_skip_text(input, ", and ")) {
    if (!try_skip_text(input, " and ")) {
      skip_text(input, ", ");
    }
  }
  return true;
}

static void parse_line(const char **const input, unsigned rock_floor[MAX_ROCKS], unsigned chip_floor[MAX_ROCKS],
                       unsigned floor, const char *const floor_name, char *rock_names[MAX_ROCKS]) {
  skip_text(input, "The ");
  skip_text(input, floor_name);
  skip_text(input, " floor contains ");
  if (**input == 'n') {
    skip_text(input, "nothing relevant.\n");
    return;
  }

  while (parse_element(input, rock_floor, chip_floor, floor, rock_names))
    ;
  skip_text(input, ".\n");
}

static struct state parse(const char *input) {
  struct state state;
  state.steps = 0;
  state.elevator_floor = 0;

  char *rock_names[MAX_ROCKS];
  for (unsigned rock = 0; rock < MAX_ROCKS; rock++) {
    rock_names[rock] = NULL;
  }

  for (unsigned floor = 0; floor < FLOORS; floor++) {
    parse_line(&input, state.rock_floor, state.chip_floor, floor, floor_names[floor], rock_names);
  }

  for (unsigned rock = 0; rock < MAX_ROCKS; rock++) {
    free(rock_names[rock]);
  }
  return state;
}

static bool is_state_finished(const struct state *const state) {
  const unsigned last_floor = FLOORS - 1;

  if (state->elevator_floor != last_floor) {
    return false;
  }
  for (unsigned i = 0; i < total_rocks; i++) {
    if (state->rock_floor[i] != last_floor || state->chip_floor[i] != last_floor) {
      return false;
    }
  }
  return true;
}

static bool floor_would_be_safe(const struct state *const state, unsigned floor) {
  bool any_rocks = false;
  bool any_lone_chips = false;
  for (unsigned rock = 0; rock < total_rocks; rock++) {
    unsigned chip = rock;
    bool rock_in_floor = state->rock_floor[rock] == floor;
    bool chip_in_floor = state->chip_floor[chip] == floor;
    any_rocks |= rock_in_floor;
    any_lone_chips |= chip_in_floor && !rock_in_floor;
  }
  return !any_lone_chips || !any_rocks;
}

static bool state_would_be_safe(const struct state *const state, unsigned prev_floor) {
  return floor_would_be_safe(state, state->elevator_floor) && floor_would_be_safe(state, prev_floor);
}

static size_t populate_possible_next_states(const struct state *const state, struct state *const next) {
  size_t count = 0;

  // for each possible elevator direction (down or up, 0 or 1)
  for (unsigned elevator_updown = 0; elevator_updown <= 1; elevator_updown++) {
    if ((state->elevator_floor == 0 && elevator_updown == 0) ||
        (state->elevator_floor == FLOORS - 1 && elevator_updown == 1)) {
      continue; // skip goin above top floor or below bottom floor
    }

    unsigned next_floor;
    if (elevator_updown == 0) {
      next_floor = state->elevator_floor - 1;
    } else {
      next_floor = state->elevator_floor + 1;
    }

    // handle taking a single rock
    for (unsigned rock = 0; rock < total_rocks; rock++) {
      if (state->rock_floor[rock] != state->elevator_floor) {
        continue; // skip rocks not in this floor
      }

      next[count] = *state;
      struct state *const next_state = next + count;
      next_state->steps += 1;
      next_state->elevator_floor = next_floor;
      next_state->rock_floor[rock] = next_floor;
      if (state_would_be_safe(next_state, state->elevator_floor)) {
        count++;
      }
    }

    // handle taking a single chip
    for (unsigned chip = 0; chip < total_rocks; chip++) {
      if (state->chip_floor[chip] != state->elevator_floor) {
        continue; // skip chips not in this floor
      }

      next[count] = *state;
      struct state *const next_state = next + count;
      next_state->steps += 1;
      next_state->elevator_floor = next_floor;
      next_state->chip_floor[chip] = next_floor;
      if (state_would_be_safe(next_state, state->elevator_floor)) {
        count++;
      }
    }

    // handle taking each pair of rocks+chips

    // how this double for loop works:
    // imagine rocks and chips are placed interclated
    // R1 C1 R2 C2 R3 C3 ....
    // then, iterate over them all linearly regardless of what they are
    // iterate again inside, starting from the next one
    // this way we get all possible combinations of 2 without repetitions
    // we're pointing at a rock if the index is divisible by 2, otherwise it's a chip
    // to get the rock index, divide by 2
    // to get the chip index, subtract one then divide by 2

    for (unsigned i = 0; i < 2 * total_rocks; i++) {
      bool first_is_rock = i % 2 == 0;
      unsigned first = (i - !first_is_rock) / 2;

      if ((first_is_rock && state->rock_floor[first] != state->elevator_floor) ||
          (!first_is_rock && state->chip_floor[first] != state->elevator_floor)) {
        continue; // skip if not in this floor
      }

      for (unsigned j = i + 1; j < 2 * total_rocks; j++) {
        bool second_is_rock = j % 2 == 0;
        unsigned second = (j - !second_is_rock) / 2;

        if ((second_is_rock && state->rock_floor[second] != state->elevator_floor) ||
            (!second_is_rock && state->chip_floor[second] != state->elevator_floor)) {
          continue; // skip if not in this floor
        }

        next[count] = *state;
        struct state *const next_state = next + count;
        next_state->steps += 1;
        next_state->elevator_floor = next_floor;
        if (first_is_rock) {
          next_state->rock_floor[first] = next_floor;
        } else {
          next_state->chip_floor[first] = next_floor;
        }
        if (second_is_rock) {
          next_state->rock_floor[second] = next_floor;
        } else {
          next_state->chip_floor[second] = next_floor;
        }
        if (state_would_be_safe(next_state, state->elevator_floor)) {
          count++;
        }
      }
    }
  }

  return count;
}

static unsigned find_fastest_path(const struct state *const init) {
  size_t size = sizeof(bool) * POSSIBLE_STATES;
  bool *seen_states = malloc(size);
  memset(seen_states, 0, size);
  set_true_for_packed_equivalent_states(init, seen_states);

  struct state *queue = malloc(sizeof(*queue) * QUEUE_SIZE);
  queue[0] = *init;
  size_t qhead = 1;
  size_t qtail = 0;

  while (qhead != qtail) {
    const struct state *const state = &queue[qtail++];
    qtail %= QUEUE_SIZE;

    // if all next states were possible we'd have...
    // total_rocks states from taking each possible rock
    // plus total_rocks states from taking each possible chip
    // plus (2*total_rocks)*(2*total_rocks-1)/2 states from taking each possible combination of 1 rock and 1 chip
    // (n over 2 is n*(n-1)/2)
    // multiplied by two as we can choose either of the two floors
    struct state possible_next_states[(total_rocks + total_rocks + (2 * total_rocks) * (2 * total_rocks - 1) / 2) * 2];
    size_t number_next_states = populate_possible_next_states(state, possible_next_states);

    for (size_t i = 0; i < number_next_states; i++) {
      size_t packed = state_packed(possible_next_states + i);
      if (seen_states[packed]) {
        continue;
      }
      set_true_for_packed_equivalent_states(possible_next_states + i, seen_states);

      if (is_state_finished(possible_next_states + i)) {
        print_state(possible_next_states + i);
        free(queue);
        free(seen_states);
        return possible_next_states[i].steps;
      }
      queue[qhead++] = possible_next_states[i];
      qhead %= QUEUE_SIZE;
      ASSERT(qhead != qtail, "queue full (s:%u)", state->steps);
    }
  }

  free(queue);
  free(seen_states);
  FAIL("could not find a final state");
}

static void solution1(const char *const input, char *const output) {
  total_rocks = 5;
  struct state init = parse(input);
  print_state(&init);
  unsigned steps = find_fastest_path(&init);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", steps);
}

static void solution2(const char *const input, char *const output) {
  total_rocks = 7;
  struct state init = parse(input);
  init.rock_floor[5] = 0;
  init.chip_floor[5] = 0;
  init.rock_floor[6] = 0;
  init.chip_floor[6] = 0;
  print_state(&init);
  unsigned steps = find_fastest_path(&init);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", steps);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
