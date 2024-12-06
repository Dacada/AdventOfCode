#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAXDECKSIZE 51

// deque (pronounced deck ;)) implementation
// header:

typedef bool (*deque_cb)(int, size_t, void *);

struct deque {
  int data[MAXDECKSIZE];
  size_t head, tail;
};

static void deque_init(struct deque *const deque);
static void deque_copyinit(struct deque *const deque, const struct deque *const other, const size_t count);

static void deque_pushback(struct deque *const deque, const int element);
static int deque_popfront(struct deque *const deque);
static int deque_peekat(const struct deque *const deque, size_t idx);

static size_t deque_length(const struct deque *const deque);
static bool deque_full(const struct deque *const deque);
static bool deque_empty(const struct deque *const deque);

static void deque_iter(const struct deque *const deque, deque_cb fun, void *args);
static bool deque_equal(const struct deque *const deque1, const struct deque *const deque2);

static void deque_print(const struct deque *const deque);

// implementations:

static void deque_init(struct deque *const deque) { deque->head = deque->tail = 0; }

static void deque_pushback(struct deque *const deque, const int element) {
  ASSERT(!deque_full(deque), "push to full deque");
  deque->data[deque->tail] = element;
  deque->tail = (deque->tail + 1) % MAXDECKSIZE;
}

static int deque_popfront(struct deque *const deque) {
  ASSERT(!deque_empty(deque), "pop from empty queue");
  int ret = deque->data[deque->head];
  deque->head = (deque->head + 1) % MAXDECKSIZE;
  return ret;
}

static size_t deque_length(const struct deque *const deque) {
  if (deque->head <= deque->tail) {
    return deque->tail - deque->head;
  } else {
    // # -> element
    // _ -> "empty"
    //
    //          5       9
    //         tail    head
    //           v       v
    // # # # # # _ _ _ _ # # #
    // |________|       |_____|
    //       5           12-9=3
    //           5  +  3
    //              8
    return deque->tail + (MAXDECKSIZE - deque->head);
  }
}

static int deque_peekat(const struct deque *const deque, size_t idx) {
  ASSERT(idx < deque_length(deque), "peek out of range");
  return deque->data[(deque->head + idx) % MAXDECKSIZE];
}

static bool deque_full(const struct deque *const deque) { return (deque->tail + 1) % MAXDECKSIZE == deque->head; }

static bool deque_empty(const struct deque *const deque) { return deque->tail == deque->head; }

static void deque_iter(const struct deque *const deque, deque_cb fun, void *args) {
  for (size_t i = 0; i < MAXDECKSIZE; i++) {
    size_t ii = (i + deque->head) % MAXDECKSIZE;
    if (ii == deque->tail) {
      break;
    }
    if (!fun(deque->data[ii], i, args)) {
      break;
    }
  }
}

struct dequecb_equal_args {
  const struct deque *other;
  bool all;
};
static bool dequecb_equal(int element, size_t idx, void *args_ptr) {
  (void)idx;
  struct dequecb_equal_args *args = args_ptr;

  args->all &= element == deque_peekat(args->other, idx);
  return args->all;
}
static bool deque_equal(const struct deque *const deque1, const struct deque *const deque2) {
  if (deque_length(deque1) != deque_length(deque2)) {
    return false;
  }

  struct dequecb_equal_args args;
  args.other = deque2;
  args.all = true;
  deque_iter(deque1, dequecb_equal, &args);
  return args.all;
}

struct dequecb_copyElement_args {
  struct deque *const target;
  size_t elementsLeft;
};
static bool dequecb_copyElement(int element, size_t idx, void *args_ptr) {
  (void)idx;
  struct dequecb_copyElement_args *args = args_ptr;
  if (args->elementsLeft == 0) {
    return false;
  }

  deque_pushback(args->target, element);

  args->elementsLeft -= 1;
  if (args->elementsLeft == 0) {
    return false;
  }

  return true;
}

static void deque_copyinit(struct deque *const deque, const struct deque *const other, const size_t count) {
  deque_init(deque);
  struct dequecb_copyElement_args args = {
      .target = deque,
      .elementsLeft = count,
  };
  deque_iter(other, dequecb_copyElement, &args);
}

#ifdef DEBUG
static bool dequecb_printElement(int element, size_t idx, void *args) {
  (void)args;
  (void)idx;
  fprintf(stderr, "%d, ", element);
  return true;
}
static void deque_print(const struct deque *const deque) {
  fprintf(stderr, "\t");
  deque_iter(deque, dequecb_printElement, NULL);
  fprintf(stderr, "\n");
}
#else
static void deque_print(const struct deque *const deque) { (void)deque; }
#endif

// end of deque

static int parse_int(const char **const i) {
  int n = 0;
  char c;
  while (isdigit(c = **i)) {
    n = n * 10 + c - '0';
    *i += 1;
  }
  return n;
}

static void parse(const char *input, struct deque *const deck1, struct deque *const deck2) {
  const char *const expect1 = "Player 1:\n";
  const size_t len1 = strlen(expect1);
  ASSERT(strncmp(expect1, input, len1) == 0, "parse error");
  input += len1;

  for (int i = 0;; i++) {
    deque_pushback(deck1, parse_int(&input));
    ASSERT(*input == '\n', "parse error");
    input++;
    if (*input == '\n') {
      break;
    }
  }

  const char *const expect2 = "\nPlayer 2:\n";
  const size_t len2 = strlen(expect2);
  ASSERT(strncmp(expect2, input, len2) == 0, "parse error");
  input += len2;

  for (int i = 0;; i++) {
    deque_pushback(deck2, parse_int(&input));
    ASSERT(*input == '\n', "parse error");
    input++;
    if (*input == '\0') {
      break;
    }
  }
}

struct calcScore_args {
  const size_t length;
  int total;
};

static bool calculate_score(int card, size_t position, void *args_ptr) {
  struct calcScore_args *args = args_ptr;

  size_t back_position = args->length - position;
  int value = card * back_position;
  int newtotal = args->total + value;

  DBG("%d * %lu = %d || +%d = %d", card, back_position, value, args->total, newtotal);
  args->total = newtotal;
  return true;
}

static void solution1(const char *const input, char *const output) {
  struct deque deck1;
  struct deque deck2;

  deque_init(&deck1);
  deque_init(&deck2);

  parse(input, &deck1, &deck2);

  for (int round = 1; !deque_empty(&deck1) && !deque_empty(&deck2); round++) {
    DBG("-- Round %d --", round);
    DBG("Player 1's deck:");
    deque_print(&deck1);
    DBG("Player 2's deck:");
    deque_print(&deck2);

    int play1 = deque_popfront(&deck1);
    int play2 = deque_popfront(&deck2);
    DBG("Player 1 plays: %d", play1);
    DBG("Player 2 plays: %d", play2);

    if (play1 > play2) {
      DBG("Player 1 wins the round!");
      deque_pushback(&deck1, play1);
      deque_pushback(&deck1, play2);
    } else {
      DBG("Player 2 wins the round!");
      deque_pushback(&deck2, play2);
      deque_pushback(&deck2, play1);
    }
    DBG("\n");
  }
  DBG("\n");
  DBG("== Post-game results ==");
  DBG("Player 1's deck:");
  deque_print(&deck1);
  DBG("Player 2's deck:");
  deque_print(&deck2);

  struct deque *winner;
  if (deque_empty(&deck1)) {
    winner = &deck2;
  } else {
    winner = &deck1;
  }

  struct calcScore_args args = {
      .length = deque_length(winner),
      .total = 0,
  };
  deque_iter(winner, calculate_score, &args);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", args.total);
}

struct gameStatus {
  struct deque *deck1_states;
  size_t capacity;
  size_t length;
};

static void gameStatus_init(struct gameStatus *const game) {
  game->capacity = 4;
  game->deck1_states = malloc(game->capacity * sizeof *game->deck1_states);
  game->length = 0;
}

static void gameStatus_free(struct gameStatus *const game) { free(game->deck1_states); }

static void gameStatus_addRound(struct gameStatus *const game, const struct deque *const deck1) {
  if (game->length >= game->capacity) {
    game->capacity *= 2;
    game->deck1_states = realloc(game->deck1_states, game->capacity * sizeof *game->deck1_states);
  }
  deque_copyinit(&game->deck1_states[game->length], deck1, deque_length(deck1));
  game->length += 1;
}

static bool gameStatus_containsRound(struct gameStatus *const game, const struct deque *const deck1) {
  for (size_t i = 0; i < game->length; i++) {
    if (deque_equal(game->deck1_states + i, deck1)) {
      return true;
    }
  }
  return false;
}

static int recursive_combat(struct deque *const deck1, struct deque *const deck2, int *const gameCount) {
  struct gameStatus status;
  gameStatus_init(&status);

  *gameCount += 1;
#ifdef DEBUG
  int thisGame = *gameCount;
#endif

  DBG("=== Game %d ===", thisGame);

  for (int roundCount = 1; !deque_empty(deck1) && !deque_empty(deck2); roundCount++) {
    DBG("-- Round %d (Game %d) --", roundCount, thisGame);
    DBG("Player 1's deck:");
    deque_print(deck1);
    DBG("Player 2's deck:");
    deque_print(deck2);

    if (gameStatus_containsRound(&status, deck1)) {
      DBG("This round is a repeat, player 1 wins instantly.");
      gameStatus_free(&status);
      return 1;
    }
    gameStatus_addRound(&status, deck1);

    unsigned play1 = deque_popfront(deck1);
    unsigned play2 = deque_popfront(deck2);
    DBG("Player 1 plays: %d", play1);
    DBG("Player 2 plays: %d", play2);

    size_t len1 = deque_length(deck1);
    size_t len2 = deque_length(deck2);

    int winner;
    if (len1 >= play1 && len2 >= play2) {
      DBG("Playing a sub-game to determine the winner...");
      struct deque subdeck1;
      struct deque subdeck2;
      deque_copyinit(&subdeck1, deck1, play1);
      deque_copyinit(&subdeck2, deck2, play2);
      winner = recursive_combat(&subdeck1, &subdeck2, gameCount);
      DBG("...anyway back to the game %d.", thisGame);
    } else {
      if (play1 > play2) {
        winner = 1;
      } else {
        winner = 2;
      }
    }

    DBG("Player %d wins round %d of game %d!", winner, roundCount, thisGame);
    if (winner == 1) {
      deque_pushback(deck1, play1);
      deque_pushback(deck1, play2);
    } else {
      deque_pushback(deck2, play2);
      deque_pushback(deck2, play1);
    }
  }

  gameStatus_free(&status);
  if (deque_empty(deck1)) {
    return 2;
  } else {
    return 1;
  }
}

static void solution2(const char *const input, char *const output) {
  struct deque deck1;
  struct deque deck2;
  deque_init(&deck1);
  deque_init(&deck2);
  parse(input, &deck1, &deck2);

  int gameCount = 0;
  int win = recursive_combat(&deck1, &deck2, &gameCount);

  struct deque *winner;
  if (win == 1) {
    winner = &deck1;
  } else {
    winner = &deck2;
  }

  struct calcScore_args args = {
      .length = deque_length(winner),
      .total = 0,
  };
  deque_iter(winner, calculate_score, &args);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", args.total);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
