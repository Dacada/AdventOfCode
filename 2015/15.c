#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>

#define NINGREDIENTS 4

#define ISNUM(n) ((n) >= '0' && (n) <= '9')
#define ZERO_IF_NEG(n) ((n) < 0 ? 0 : (n))

struct ingredient_t {
  int capacity;
  int durability;
  int flavor;
  int texture;
  int calories;
};

struct ingredient_t ingredients[NINGREDIENTS];

static int parse_next_number(const char *const input, int *const i) {
  int j = *i;
  while (!ISNUM(input[j]) && input[j] != '-') {
    j++;
  }

  bool negative = input[j] == '-';
  if (negative) {
    j++;
  }

  int num = 0;
  while (ISNUM(input[j])) {
    num = num*10 + input[j]-0x30;
    j++;
  }

  *i = j;

  if (negative) {
    return -num;
  } else {
    return num;
  }
}

static void parse_line(const char *const input, int *const i, const int j) {
  ingredients[j].capacity = parse_next_number(input, i);
  ingredients[j].durability = parse_next_number(input, i);
  ingredients[j].flavor = parse_next_number(input, i);
  ingredients[j].texture = parse_next_number(input, i);
  ingredients[j].calories = parse_next_number(input, i);
}

static void parse(const char *const input) {
  for (int i=0, j=0;; i++) {
    if (input[i] == '\0') {
      break;
    }
    
    parse_line(input, &i, j);
    j++;
    ASSERT(input[i] == '\n', "Did not parse full line");
  }
}

static void iter_shares(const unsigned int elements, const unsigned int total, unsigned int *const share, const size_t i, void(*const fun)(const unsigned int *const, void *const), void *const args) {
  if (elements > total) {
    FAIL("Attempt to call iter shares with more elements to share with than things to share");
  }
  else if (elements < 2) {
    FAIL("Attempt to call iter shares with less than two elements");
  } else if (elements == 2) {
    for (unsigned int n=1; n<total; n++) {
      const unsigned int m = total - n;
      share[i] = n;
      share[i+1] = m;
      fun(share, args);
    }
  } else {
    for (unsigned int n=1; n<total-elements+2; n++) {
      const unsigned int m = total - n;
      share[i] = n;
      iter_shares(elements - 1, m, share, i+1, fun, args);
    }
  }
}

static unsigned int calc_share_value(const unsigned int *const share) {
  int total_capacity = 0;
  int total_durability = 0;
  int total_flavor = 0;
  int total_texture = 0;
  
  for (int i=0; i<NINGREDIENTS; i++) {
    total_capacity += ingredients[i].capacity * share[i];
    total_durability += ingredients[i].durability * share[i];
    total_flavor += ingredients[i].flavor * share[i];
    total_texture += ingredients[i].texture * share[i];
  }
  
  return ZERO_IF_NEG(total_capacity) * ZERO_IF_NEG(total_durability) * ZERO_IF_NEG(total_flavor) * ZERO_IF_NEG(total_texture);
}

static unsigned int calc_share_value_if_500cal(const unsigned int *const share) {
  int total_calories = 0;
  for (int i=0; i<NINGREDIENTS; i++) {
    total_calories += ingredients[i].calories * share[i];
  }

  if (total_calories == 500) {
    return calc_share_value(share);
  } else {
    return 0;
  }
}

static void find_max_share(const unsigned int *const share, void *const args) {
  unsigned int *max_ptr = args;
  unsigned int max = *max_ptr;
  unsigned int share_val = calc_share_value(share);
  if (share_val > max)
    *max_ptr = share_val;
}

static void find_max_share_500cal(const unsigned int *const share, void *const args) {
  unsigned int *max_ptr = args;
  unsigned int max = *max_ptr;
  unsigned int share_val = calc_share_value_if_500cal(share);
  if (share_val > max)
    *max_ptr = share_val;
}

static unsigned int solve1(void) {
  unsigned int max = 0;
  unsigned int share[NINGREDIENTS];
  iter_shares(NINGREDIENTS, 100, share, 0, find_max_share, &max);
  return max;
}

static unsigned int solve2(void) {
  unsigned int max = 0;
  unsigned int share[NINGREDIENTS];
  iter_shares(NINGREDIENTS, 100, share, 0, find_max_share_500cal, &max);
  return max;
}

static void solution1(const char *const input, char *const output) {
  parse(input);
  unsigned int result = solve1();
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

static void solution2(const char *const input, char *const output) {
  parse(input);
  unsigned int result = solve2();
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
