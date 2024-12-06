#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

bool J_IS_JOKER = false;

struct hand {
  char cards[5];
  int bid;
};

static int hand_index(char c) {
  if (isdigit(c)) {
    return c - '2';
  }
  if (c == 'T') {
    return 8;
  }
  if (c == 'J') {
    return 9;
  }
  if (c == 'Q') {
    return 10;
  }
  if (c == 'K') {
    return 11;
  }
  if (c == 'A') {
    return 12;
  }
  FAIL("impossible hand symbol");
}

static int hand_type(const char cards[5]) {
  int counts[13];
  for (int i = 0; i < 13; i++) {
    counts[i] = 0;
  }
  for (int i = 0; i < 5; i++) {
    int j = hand_index(cards[i]);
    counts[j]++;
  }

  if (J_IS_JOKER) {
    int joker_index = hand_index('J');
    if (counts[joker_index] == 5) {
      return 7;
    }
    if (counts[joker_index] > 0) {
      int most_cards = -1;
      int most_cards_num = 0;
      for (int i = 0; i < 13; i++) {
        if (i == joker_index) {
          continue;
        }
        int n = counts[i];
        if (n > most_cards_num) {
          most_cards = i;
          most_cards_num = n;
        }
      }
      if (most_cards == -1) {
        FAIL("impossible");
      }
      counts[most_cards] += counts[joker_index];
      counts[joker_index] = 0;
    }
  }

  bool triplet = false;
  int pairs = 0;
  for (int i = 0; i < 13; i++) {
    if (counts[i] == 5) {
      return 7; // five of a kind
    } else if (counts[i] == 4) {
      return 6; // four of a kind
    } else if (counts[i] == 3) {
      triplet = true;
    } else if (counts[i] == 2) {
      pairs++;
    }
  }

  if (triplet) {
    if (pairs) {
      return 5; // full house
    } else {
      return 4; // three of a kind
    }
  } else if (pairs == 2) {
    return 3; // two pairs
  } else if (pairs) {
    return 2; // pair
  } else {
    return 1; // high card
  }
}

static int hand_symbol(char c) {
  if (J_IS_JOKER && c == 'J') {
    return -1;
  }
  return hand_index(c);
}

static int cmp_hand(const void *h1, const void *h2) {
  const struct hand *hand1 = h1;
  const struct hand *hand2 = h2;

  int type1 = hand_type(hand1->cards);
  int type2 = hand_type(hand2->cards);

  if (type1 != type2) {
    return type1 - type2;
  }

  for (int i = 0; i < 5; i++) {
    int sym1 = hand_symbol(hand1->cards[i]);
    int sym2 = hand_symbol(hand2->cards[i]);
    if (sym1 != sym2) {
      return sym1 - sym2;
    }
  }

  return 0;
}

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error '%d' '%c'", **input, **input);

  int n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static void parse_hand(const char **input, struct hand *hand) {
  memcpy(hand->cards, *input, 5);
  *input += 5;
  ASSERT(**input == ' ', "parse error");
  *input += 1;
  hand->bid = parse_int(input);
  if (**input == '\n') {
    *input += 1;
  }
}

static struct hand *parse_input(const char *input, int *length) {
  int len = 0;
  int cap = 4;
  struct hand *list = malloc(sizeof(*list) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    parse_hand(&input, &list[len++]);
  }

  *length = len;
  return list;
}

static void solution(const char *const input, char *const output) {
  int len;
  struct hand *hands = parse_input(input, &len);
  qsort(hands, len, sizeof(*hands), cmp_hand);

  int res = 0;
  for (int i = 0; i < len; i++) {
    res += hands[i].bid * (i + 1);
  }

  free(hands);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

static void solution1(const char *const input, char *const output) { solution(input, output); }

static void solution2(const char *const input, char *const output) {
  J_IS_JOKER = true;
  solution(input, output);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
