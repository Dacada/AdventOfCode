#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct rule {
  size_t idx;
  bool exact;
  char value;

  size_t nsubrules[2];
  size_t subrules[8];
};

struct problem {
  struct rule *rules;
  char **messages;
  size_t messages_len;
};

static size_t parse_int(const char **const input) {
  size_t n = 0;
  char c;
  while (isdigit(c = **input)) {
    n = n * 10 + c - '0';
    *input += 1;
  }
  return n;
}

static struct rule parse_rule(const char **const input) {
  struct rule result;
  result.idx = parse_int(input);

  ASSERT(**input == ':', "parse error");
  *input += 1;
  ASSERT(**input == ' ', "parse error");
  *input += 1;

  if (**input == '"') {
    result.exact = true;
    *input += 1;
    result.value = **input;
    *input += 1;
    ASSERT(**input == '"', "parse error");
    *input += 1;

    for (int i = 0; i < 2; i++) {
      result.nsubrules[i] = 0;
    }
    for (int i = 0; i < 8; i++) {
      result.subrules[i] = 0;
    }

    return result;
  }
  result.exact = false;

  size_t i = 0;
  size_t j = 0;
  for (;;) {
    size_t n = parse_int(input);
    while (**input == ' ') {
      *input += 1;
    }

    ASSERT(i < 8, "parse error");
    result.subrules[i++] = n;

    if (**input == '|') {
      ASSERT(j < 2, "parse error");
      result.nsubrules[j++] = i;

      *input += 1;
      while (**input == ' ') {
        *input += 1;
      }
    } else if (**input == '\n') {
      break;
    }
  }
  result.nsubrules[j++] = i;
  while (j < 2) {
    result.nsubrules[j++] = 0;
  }

  return result;
}

static void parse_rules(const char **const input, struct problem *const problem) {
  size_t capacity = 16;
  problem->rules = malloc(capacity * sizeof *problem->rules);
  for (; **input != '\n'; *input += 1) {
    struct rule rule = parse_rule(input);
    ASSERT(**input == '\n', "parse error");

    bool grow = false;
    while (capacity <= rule.idx) {
      capacity *= 2;
      grow = true;
    }
    if (grow) {
      problem->rules = realloc(problem->rules, capacity * sizeof *problem->rules);
    }

    problem->rules[rule.idx] = rule;
  }
}

static void parse_messages(const char **input, struct problem *const problem) {
  size_t capacity = 32;
  problem->messages = malloc(sizeof(*problem->messages) * capacity);
  problem->messages_len = 0;

  for (; **input != '\0'; *input += 1) {
    if (problem->messages_len >= capacity) {
      capacity *= 2;
      problem->messages = realloc(problem->messages, sizeof(*problem->messages) * capacity);
    }

    const char *linebreak = strchr(*input, '\n');
    size_t len = linebreak - *input;
    problem->messages[problem->messages_len++] = strndup(*input, len);
    *input = linebreak;
  }
}

static void parse(const char *input, struct problem *const problem) {
  parse_rules(&input, problem);
  ASSERT(*input == '\n', "parse error");
  input += 1;
  parse_messages(&input, problem);
  ASSERT(*input == '\0', "parse error");
}

static int strcmp_ptr_wrapper(const void *a, const void *b) {
  const char *const *ptra = a;
  const char *const *ptrb = b;
  return strcmp(*ptra, *ptrb);
}

/*
int spaces;
static void s(void) {
        for (int i=0; i<spaces; i++) {
                fputc(' ', stderr);
        }
}
static void pm(const char **msgs, size_t len) {
        for (size_t i=0; i<len; i++) {
                fprintf(stderr, "\"%s\", ", msgs[i]);
        }
}
*/
static void rule_partial_match(const char ***msgs_ptr, size_t *const len_msgs, size_t *const cap_msgs,
                               const struct rule *const rules, const size_t idx) {
  // spaces++;

  const char **msgs = *msgs_ptr;
  const struct rule rule = rules[idx];

  /*
  s();
  fprintf(stderr, "rule_partial_match");
  fputc('(', stderr);
  pm(msgs, *len_msgs);
  fprintf(stderr, "rule=%lu", idx);
  fputc(')', stderr);
  fputc('\n', stderr);
  */

  // We will construct a list of all possible ways the messages can end up
  // after consuming all valid rules
  const char **new_msgs = malloc(*cap_msgs * sizeof *new_msgs);
  size_t new_msgs_len = 0;
  size_t new_msgs_cap = *cap_msgs;

  if (rule.exact) {
    // s(); fprintf(stderr, "exact matches\n");

    // If the rule is exact, filter out the list such that only
    // the matching messages remain.
    for (size_t i = 0; i < *len_msgs; i++) {
      const char *msg = msgs[i];
      // s(); fprintf(stderr, "\"%s\"", msg);

      if (*msg == rule.value) {
        // fprintf(stderr, " -> %c == %c | ok!\n", *msg, rule.value);
        if (new_msgs_len >= new_msgs_cap) {
          new_msgs_cap *= 2;
          new_msgs = realloc(new_msgs, new_msgs_cap * sizeof *new_msgs);
        }
        new_msgs[new_msgs_len++] = msg + 1;
      } else {
        // fprintf(stderr, " -> %c != %c | not ok...\n", *msg, rule.value);
      }
    }
  } else {
    // Otherwise, copy the list and recurse. This recursion will
    // modify the copy. If we have two sets of rules, we will make
    // two copies and pass them both through both sets of
    // rules. Afterwards we will perform a union operation between
    // both copies, such that they merge and only unique substrings
    // remain.

    size_t cap_submsgs1 = *cap_msgs;
    size_t len_submsgs1 = *len_msgs;
    const char **submsgs1 = malloc(cap_submsgs1 * sizeof *new_msgs);
    memcpy(submsgs1, msgs, len_submsgs1 * sizeof *new_msgs);

    for (size_t i = 0; i < rule.nsubrules[0]; i++) {
      rule_partial_match(&submsgs1, &len_submsgs1, &cap_submsgs1, rules, rule.subrules[i]);
      if (len_submsgs1 == 0) {
        break;
      }
    }

    size_t cap_submsgs2;
    size_t len_submsgs2 = 0;
    const char **submsgs2 = NULL;
    if (rule.nsubrules[1] > 0) {
      cap_submsgs2 = *cap_msgs;
      len_submsgs2 = *len_msgs;
      submsgs2 = malloc(cap_submsgs2 * sizeof *new_msgs);
      memcpy(submsgs2, msgs, len_submsgs2 * sizeof *new_msgs);

      for (size_t i = rule.nsubrules[0]; i < rule.nsubrules[1]; i++) {
        rule_partial_match(&submsgs2, &len_submsgs2, &cap_submsgs2, rules, rule.subrules[i]);
        if (len_submsgs2 == 0) {
          break;
        }
      }
    }

    if (len_submsgs1 > 1) {
      qsort(submsgs1, len_submsgs1, sizeof *submsgs1, strcmp_ptr_wrapper);
    }
    if (len_submsgs2 > 1) {
      qsort(submsgs2, len_submsgs2, sizeof *submsgs2, strcmp_ptr_wrapper);
    }

    bool grow;
    while (new_msgs_cap < (len_submsgs1 + len_submsgs2)) {
      new_msgs_cap *= 2;
      grow = true;
    }
    if (grow) {
      new_msgs = realloc(new_msgs, new_msgs_cap * sizeof *new_msgs);
    }

    size_t i = 0, j = 0;
    while (i < len_submsgs1 && j < len_submsgs2) {
      const char *msg1 = submsgs1[i];
      const char *msg2 = submsgs2[j];

      int cmp = strcmp(msg1, msg2);
      if (cmp <= 0) {
        new_msgs[new_msgs_len++] = msg1;
        i++;
        if (cmp == 0) {
          j++;
        }
      } else if (cmp > 0) {
        new_msgs[new_msgs_len++] = msg2;
        j++;
      }
    }
    while (i < len_submsgs1) {
      new_msgs[new_msgs_len++] = submsgs1[i++];
    }
    while (j < len_submsgs2) {
      new_msgs[new_msgs_len++] = submsgs2[j++];
    }

    free(submsgs1);
    free(submsgs2);
  }

  free(msgs);
  *msgs_ptr = new_msgs;
  *len_msgs = new_msgs_len;
  *cap_msgs = new_msgs_cap;
  // spaces--;
}

static bool rule_match(const struct rule *const rules, const size_t idx, const char *msg) {
  size_t capacity = 8;
  size_t length = 1;
  const char **msgs = malloc(capacity * sizeof *msgs);
  msgs[0] = msg;

  rule_partial_match(&msgs, &length, &capacity, rules, idx);

  bool any = false;
  for (size_t i = 0; i < length; i++) {
    if (*(msgs[i]) == '\0') {
      any = true;
      break;
    }
  }

  free(msgs);
  return any;
}

static void solution(const char *const input, char *const output, bool alter) {
  struct problem problem;
  parse(input, &problem);

  if (alter) {
    problem.rules[8].nsubrules[0] = 1;
    problem.rules[8].nsubrules[1] = 3;
    problem.rules[8].subrules[0] = 42;
    problem.rules[8].subrules[1] = 42;
    problem.rules[8].subrules[2] = 8;

    problem.rules[11].nsubrules[0] = 2;
    problem.rules[11].nsubrules[1] = 5;
    problem.rules[11].subrules[0] = 42;
    problem.rules[11].subrules[1] = 31;
    problem.rules[11].subrules[2] = 42;
    problem.rules[11].subrules[3] = 11;
    problem.rules[11].subrules[4] = 31;
  }

  for (size_t i = 0; i < 5; i++) {
    struct rule rule = problem.rules[i];
    DBG("rule %lu:", rule.idx);
    if (rule.exact) {
      DBG("exact: \"%c\"", rule.value);
    } else {
      for (size_t j = 0; j < rule.nsubrules[0]; j++) {
        DBG("subrule %lu", rule.subrules[j]);
      }
      DBG("also");
      for (size_t j = rule.nsubrules[0]; j < rule.nsubrules[1]; j++) {
        DBG("subrule %lu", rule.subrules[j]);
      }
    }
    DBG("---");
  }

  int count = 0;
  for (size_t i = 0; i < problem.messages_len; i++) {
    const char *message = problem.messages[i];
    if (rule_match(problem.rules, 0, message)) {
      DBG("%s yes", message);
      count++;
    } else {
      DBG("%s no", message);
    }
    free(problem.messages[i]);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
  free(problem.rules);
  free(problem.messages);
}

static void solution1(const char *const input, char *const output) { solution(input, output, false); }

static void solution2(const char *const input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
