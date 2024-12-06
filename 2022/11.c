#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_ITEMS 128

// #define DBGLOG
#if defined(DEBUG) && defined(DBGLOG)
#define MONKEYLOG(msg, ...) fprintf(stderr, msg "\n", ##__VA_ARGS__)
#else
#define MONKEYLOG(msg, ...)
#endif

// cannot use the method for part 2 with part 1 because math
static bool part1;

static int intcmp(const void *a, const void *b) { return *(const int *)a - *(const int *)b; }

enum operation {
  OP_ADDITION,
  OP_MULTIPLY,
  OP_POWER2,
};

#define NPRIMES 8
// given a prime, get an index
//                     0   1  2  3   4  5   6  7   8   9  10 11  12 13  14  15  16 17  18 19
const int primes[] = {-1, -1, 0, 1, -1, 2, -1, 3, -1, -1, -1, 4, -1, 5, -1, -1, -1, 6, -1, 7};
// given an index, get a prime
const int primeIdx[NPRIMES] = {2, 3, 5, 7, 11, 13, 17, 19};
struct number {
  int n; // need to operate with the actual number for part 1 because 3
         // doesn't have a multiplicative inverse mod every prime
  int modPrime[NPRIMES];
};

static struct number *create_number(int x) {
  struct number *r = malloc(sizeof(*r));
  r->n = x;
  for (int i = 0; i < NPRIMES; i++) {
    r->modPrime[i] = x % primeIdx[i];
  }
  return r;
}

#if defined(DEBUG) && defined(DBGLOG)
static int dbg_number_to_int(const struct number *n) { return n->n; }
#endif

static bool number_divisible(struct number *n, int d) {
  if (part1) {
    return n->n % d == 0;
  }

  ASSERT(d <= 19, "%d not a prime", d);
  int i = primes[d];
  ASSERT(i >= 0, "%d not a prime", d);
  return n->modPrime[i] == 0;
}

static void number_operate(struct number *n, enum operation op, int arg) {
  if (part1) {
    switch (op) {
    case OP_ADDITION:
      n->n += arg;
      break;
    case OP_MULTIPLY:
      n->n *= arg;
      break;
    case OP_POWER2:
      n->n *= n->n;
      break;
    default:
      FAIL("invalid op %d", op);
    }
    return;
  }

  for (int i = 0; i < NPRIMES; i++) {
    switch (op) {
    case OP_ADDITION:
      n->modPrime[i] += arg;
      break;
    case OP_MULTIPLY:
      n->modPrime[i] *= arg;
      break;
    case OP_POWER2:
      n->modPrime[i] *= n->modPrime[i];
      break;
    default:
      FAIL("invalid op %d", op);
    }
    n->modPrime[i] %= primeIdx[i];
  }
}

static void number_div3(struct number *n) {
  ASSERT(part1, "cannot do div 3 in part 2");
  n->n /= 3;
}

struct monkey {
  struct number *items[MAX_ITEMS];
  int nitems;
  enum operation operation;
  int operation_arg;
  int test_args[3];
};

static void skip_newlines(const char **input) {
  while (**input == '\n') {
    *input += 1;
  }
}

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error");
  int r = 0;
  while (isdigit(**input)) {
    r *= 10;
    r += **input - '0';
    *input += 1;
  }
  return r;
}

static void monkey_free(struct monkey *monkey) {
  for (int i = 0; i < monkey->nitems; i++) {
    free(monkey->items[i]);
  }
}

#define ASSERT_STRING(input, str)                                                                                      \
  ASSERT(strncmp(input, str, sizeof(str) - 1) == 0, "parse error");                                                    \
  input += sizeof(str) - 1;

static void parse_monkey(const char **const input, struct monkey *monkey) {
  ASSERT_STRING(*input, "Monkey ");
  parse_int(input);

  ASSERT_STRING(*input, ":\n  Starting items: ");
  monkey->nitems = 0;
  for (;;) {
    int n = parse_int(input);
    monkey->items[monkey->nitems++] = create_number(n);
    if (**input != ',') {
      break;
    }
    ASSERT_STRING(*input, ", ");
  }

  ASSERT_STRING(*input, "\n  Operation: new = old ");
  if (**input == '*') {
    *input += 2;
    if (**input == 'o') {
      ASSERT_STRING(*input, "old");
      monkey->operation = OP_POWER2;
    } else {
      int n = parse_int(input);
      monkey->operation = OP_MULTIPLY;
      monkey->operation_arg = n;
    }
  } else if (**input == '+') {
    *input += 2;
    int n = parse_int(input);
    monkey->operation = OP_ADDITION;
    monkey->operation_arg = n;
  } else {
    FAIL("parse error");
  }

  ASSERT_STRING(*input, "\n  Test: divisible by ");
  monkey->test_args[0] = parse_int(input);
  ASSERT_STRING(*input, "\n    If true: throw to monkey ");
  monkey->test_args[1] = parse_int(input);
  ASSERT_STRING(*input, "\n    If false: throw to monkey ");
  monkey->test_args[2] = parse_int(input);
}

static int parse_input(const char *input, struct monkey **monkeys) {
  int len = 0;
  int cap = 8;
  *monkeys = malloc(sizeof(**monkeys) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      *monkeys = realloc(*monkeys, sizeof(**monkeys) * cap);
    }

    struct monkey *monkey = *monkeys + len;
    parse_monkey(&input, monkey);
    len++;

    skip_newlines(&input);
  }

  return len;
}

static int test_item(const struct monkey *monkey, struct number *n) {
  int div = monkey->test_args[0];
  int yes = monkey->test_args[1];
  int no = monkey->test_args[2];

  int next;
  char *str;
  if (number_divisible(n, div)) {
    next = yes;
    str = "";
  } else {
    next = no;
    str = " not";
  }
  (void)str;
  MONKEYLOG("    Current worry level is%s divisible by %d", str, div);
  MONKEYLOG("    Item with worry level %d is thrown to monkey %d", dbg_number_to_int(n), next);
  return next;
}

static void add_item(struct monkey *monkey, struct number *item) {
  ASSERT(monkey->nitems < MAX_ITEMS - 1, "too many items");
  monkey->items[monkey->nitems] = item;
  monkey->nitems++;
}

static int inspect_item(struct monkey *monkey, int i) {
  struct number *item = monkey->items[i];
  MONKEYLOG("  Monkey inspects an item with a worry level of %d", dbg_number_to_int(item));
  number_operate(item, monkey->operation, monkey->operation_arg);
  if (part1) {
    number_div3(item);
  }
  MONKEYLOG("    Monkey gets bored with item. "
            "Worry level is divided by 3 to %d",
            dbg_number_to_int(item));
  int next = test_item(monkey, item);
  return next;
}

static int monkey_turn(struct monkey *monkeys, int i) {
  MONKEYLOG("Monkey %d", i);
  struct monkey *monkey = monkeys + i;
  for (int j = 0; j < monkey->nitems; j++) {
    int k = inspect_item(monkey, j);
    add_item(&monkeys[k], monkey->items[j]);
    monkey->items[j] = NULL;
  }
  int r = monkey->nitems;
  monkey->nitems = 0;
  return r;
}

static void solution(const char *const input, char *const output, int turns) {
  struct monkey *monkeys;
  int len = parse_input(input, &monkeys);

  int *totals = calloc(len, sizeof(*totals));
  for (int turn = 0; turn < turns; turn++) {
    bool dbgPrnt = turn == 1 || turn == 20 || turn % 1000 == 0;
    if (dbgPrnt)
      DBG("== After round %d ==", turn);
    for (int i = 0; i < len; i++) {
      totals[i] += monkey_turn(monkeys, i);
      if (dbgPrnt)
        DBG("Monkey %d inspected items %d times.", i, totals[i]);
    }
    if (dbgPrnt)
      DBG(" ");
  }
  qsort(totals, len, sizeof(*totals), intcmp);

  long total = (long)totals[len - 1] * (long)totals[len - 2];
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", total);
  for (int i = 0; i < len; i++) {
    monkey_free(&monkeys[i]);
  }
  free(monkeys);
  free(totals);
}

static void solution1(const char *const input, char *const output) {
  part1 = true;
  solution(input, output, 20);
}

static void solution2(const char *const input, char *const output) {
  part1 = false;
  solution(input, output, 10000);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
