#include <aoclib.h>

#include <ctype.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

// #define DEBUG

#define IS_VAR(c) ((c) >= 'a' && (c) <= 'z')
#define IS_NUM(c) ((c) >= '0' && (c) <= '9')
#define IS_OP(c) ((c) >= 'A' && (c) <= 'Z')

typedef enum param_type_t { NONE, NUMBER, VARIABLE } param_type_t;

typedef enum operation_t { NOT, LSHIFT, RSHIFT, AND, OR, EQ } operation_t;

typedef struct expression_t {
  uint16_t param1;
  uint16_t param2;
  param_type_t param1_type;
  param_type_t param2_type;
  operation_t op;
} expression_t;

expression_t expressions[512];

uint16_t known_val[1024];   // index: var, content: value
bool is_known[1024];        // index: var, content: the value in known_val is correct for this var
uint16_t needs[1024][1024]; // index: var, content: vars which depend on index var in order to be calculated, they might
                            // depend on more vars. this array is laid out such that index 0 is the number of vars in
                            // the array and the following elements are the vars themselves
expression_t *unknown[1024]; // index: var, content: expression to calculate var or NULL

static size_t skip_whitespace(const char *const text, size_t i) {
  while (isspace(text[i]))
    i++;
  return i;
}

static uint16_t encode_var(const char c1, const char c2) {
  if (isspace(c2)) {
    return (uint16_t)(c1 - 'a' + 1) & 0x1F;
  } else {
    return (((uint16_t)(c2 - 'a' + 1) & 0x1F) << 5) | ((uint16_t)(c1 - 'a' + 1) & 0x1F);
  }
}

/*
static char decode_var(uint16_t var, char *c2) {
  if (var <= 26) {
    return (char)(var + 'a' - 1);
  } else {
    *c2 = (char)((var & (0x1F << 5) >> 5) + 'a' - 1);
    return (char)((var & 0x1F) + 'a' - 1);
  }
}
*/

static size_t eval_expr(const char *const text, size_t i, size_t j, uint16_t *const val, bool *const ok) {
  (void)j; // used only in debug mode, suppress warnings otherwise

  if (IS_VAR(text[i])) {
    const uint16_t var = encode_var(text[i], text[i + 1]);
    if (is_known[var]) {
      *ok = true;
      *val = known_val[var];
    } else {
      *ok = false;
      *val = var;
    }
    return i + 2;
  } else if (IS_NUM(text[i])) {
    uint16_t num = 0;
    while (IS_NUM(text[i])) {
      num = num * 10 + (text[i] - 0x30);
      i++;
    }
    *ok = true;
    *val = num;
    return i;
  } else {
    FAIL("Attempt to eval but wasn't num or var");
  }
}

static uint16_t apply_op(const uint16_t v1, const uint16_t v2, const operation_t op) {
  switch (op) {
  case NOT:
    return ~v1;
  case LSHIFT:
    return v1 << v2;
  case RSHIFT:
    return v1 >> v2;
  case AND:
    return v1 & v2;
  case OR:
    return v1 | v2;
  case EQ:
    return v1;
  default:
    FAIL("Unexpected operation to apply!");
  }
}

static bool assign_value(const uint16_t var, const uint16_t val) {
  known_val[var] = val;
  is_known[var] = true;
  unknown[var] = NULL;

  if (var == encode_var('a', ' ')) {
    return true;
  }

  // iterate each dependency of the newly assigned var

  // variables we need to keep track of stuff
  uint16_t *vars_found = malloc(1024 * sizeof(uint16_t));  // dependencies we have found a value for
  uint16_t *vars_values = malloc(1024 * sizeof(uint16_t)); // value we have found for a dependency
  int removed_elements = 0; // pointer to next value to assign in vars_found and vars_values

  for (int i = 0; i < needs[var][0]; i++) {
    const uint16_t dep = needs[var][i + 1];

    expression_t *const expr = unknown[dep];
    if (expr != NULL) {
      if (expr->param1_type == VARIABLE && expr->param1 == var) {
        expr->param1_type = NUMBER;
        expr->param1 = val;
      }

      if (expr->param2_type == VARIABLE && expr->param2 == var) {
        expr->param2_type = NUMBER;
        expr->param2 = val;
      }

      if (expr->param1_type == NUMBER && (expr->param2_type == NUMBER || expr->param2_type == NONE)) {
        const uint16_t newval = apply_op(expr->param1, expr->param2, expr->op);
        vars_found[removed_elements] = dep;
        vars_values[removed_elements] = newval;
        removed_elements++;
      }
    }
  }
  needs[var][0] = 0;

  bool retval;

  for (int i = 0; i < removed_elements; i++) {
    if (assign_value(vars_found[i], vars_values[i])) {
      retval = true;
      goto end;
    }
  }
  retval = false;

end:
  free(vars_found);
  free(vars_values);

  return retval;
}

static void assign_dependency(const uint16_t depends, const uint16_t is_depended) {
  const uint16_t len = needs[is_depended][0] + 1;
  needs[is_depended][len] = depends;
  needs[is_depended][0] = len;
}

static size_t parse_assign(const char *const text, size_t i, size_t j, expression_t *const expr, const uint16_t val,
                           const bool isnum) {
  (void)j; // used only in debug mode, suppress warnings otherwise

  ASSERT(IS_VAR(text[i]), "Expected lowercase character but got something else");
  const uint16_t var = encode_var(text[i], text[i + 1]);
  if (isspace(text[i + 1])) {
    i += 1;
  } else {
    i += 2;
  }

  // Solution 2 breaks this assert
  // ASSERT(!known_val[var], "The destination of a rule is a known value");
  // So we replace it with this if.
  if (!known_val[var]) {
    if (isnum) {
      if (assign_value(var, val)) {
        return 0;
      }
    } else {
      assign_dependency(var, val);
      expr->param1 = val;
      expr->param1_type = VARIABLE;
      expr->param2_type = NONE;
      expr->op = EQ;
      unknown[var] = expr;
    }
  }

  return i;
}

#ifdef DEBUG

static void assert_op(const char *const text, const size_t i, const size_t j, const char *const op_text) {
  for (int k = 0;; k++) {
    const char c = op_text[k];
    if (c == '\0') {
      break;
    }

    const char d = text[i + k];
    if (d == '\0') {
      break;
    }

    ASSERT(c == d, "Unexpected operation name");
  }
}

#else

static void assert_op(const char *const text, size_t i, const size_t j, const char *const op_text) {
  (void)text;
  (void)i;
  (void)j;
  (void)op_text;
}

#endif

static size_t parse_op(const char *const text, size_t i, size_t j, expression_t *const expr, const uint16_t val1,
                       const bool isnum1) {
  operation_t op;
  switch (text[i]) {
  case 'L':
    i += 1;
    assert_op(text, i, j, "SHIFT");
    op = LSHIFT;
    i += 6;
    break;
  case 'R':
    i += 1;
    assert_op(text, i, j, "SHIFT");
    op = RSHIFT;
    i += 6;
    break;
  case 'A':
    i += 1;
    assert_op(text, i, j, "ND");
    op = AND;
    i += 3;
    break;
  case 'O':
    i += 1;
    assert_op(text, i, j, "R");
    op = OR;
    i += 2;
    break;
  default:
    FAIL("Unknown operation");
  }

  i = skip_whitespace(text, i);

  uint16_t val2;
  bool isnum2;
  i = eval_expr(text, i, j, &val2, &isnum2);

  i = skip_whitespace(text, i);

  ASSERT(text[i] == '-' && text[i + 1] == '>', "Expected arrow (->) but didn't find it");
  i += 2;

  i = skip_whitespace(text, i);

  ASSERT(IS_VAR(text[i]), "Expected lowercase letter but found something else");
  const uint16_t var = encode_var(text[i], text[i + 1]);
  if (isspace(text[i + 1])) {
    i += 1;
  } else {
    i += 2;
  }

  ASSERT(!known_val[var], "The destination rule is a known rule");
  if (isnum1 && isnum2) {
    const uint16_t val = apply_op(val1, val2, op);
    if (assign_value(var, val)) {
      return 0;
    }
  } else {
    expr->param1 = val1;
    if (isnum1) {
      expr->param1_type = NUMBER;
    } else {
      assign_dependency(var, val1);
      expr->param1_type = VARIABLE;
    }

    expr->param2 = val2;
    if (isnum2) {
      expr->param2_type = NUMBER;
    } else {
      assign_dependency(var, val2);
      expr->param2_type = VARIABLE;
    }

    expr->op = op;

    unknown[var] = expr;
  }

  return i;
}

static size_t parse_assign_or_op(const char *const text, size_t i, const size_t j, expression_t *const expr) {
  uint16_t val;
  bool isnum;
  i = eval_expr(text, i, j, &val, &isnum);

  i = skip_whitespace(text, i);

  if (text[i] == '-') {
    ASSERT(text[i + 1] == '>', "Expected an arrow (->) but found something else");
    i += 2;

    i = skip_whitespace(text, i);

    return parse_assign(text, i, j, expr, val, isnum);
  } else if (IS_OP(text[i])) {
    return parse_op(text, i, j, expr, val, isnum);
  } else {
    FAIL("Expected an arrow or an operation but got neither");
  }
}

static size_t parse_not(const char *const text, size_t i, const size_t j, expression_t *const expr) {
  ASSERT(text[i + 1] == 'O' && text[i + 2] == 'T', "Expected NOT but it wasn't.");
  i += 3;

  i = skip_whitespace(text, i);

  uint16_t v1;
  bool known_value;
  i = eval_expr(text, i, j, &v1, &known_value);
  if (known_value) {
    v1 = ~v1;
  }

  i = skip_whitespace(text, i);

  ASSERT(text[i] == '-' && text[i + 1] == '>', "Expected arrow (->) but didn't find it");
  i += 2;

  i = skip_whitespace(text, i);

  ASSERT(IS_VAR(text[i]), "Expected lowercase letter but found something else");
  const uint16_t var = encode_var(text[i], text[i + 1]);
  if (isspace(text[i + 1])) {
    i += 1;
  } else {
    i += 2;
  }

  ASSERT(!known_val[var], "The destination of a rule is a known value");
  if (known_value) {
    assign_value(var, v1);
  } else {
    assign_dependency(var, v1);
    expr->param1 = v1;
    expr->param1_type = VARIABLE;
    expr->param2_type = NONE;
    expr->op = NOT;
    unknown[var] = expr;
  }

  return i;
}

static size_t parse_line(const char *const text, size_t i, size_t j) {
  expression_t *const expr = &expressions[j];

  if (IS_VAR(text[i]) || IS_NUM(text[i])) {
    return parse_assign_or_op(text, i, j, expr);
  } else if (text[i] == 'N') {
    return parse_not(text, i, j, expr);
  } else {
    FAIL("Line doesn't start with N, a small case letter or a number.");
  }
}

static void solution(const char *const input) {
  for (size_t i = 0, j = 0; input[i] != '\0'; i++, j++) {
    i = parse_line(input, i, j);
    if (i == 0) {
      break;
    }
  }
}

static void solution1(const char *const input, char *const output) {
  solution(input);

  uint16_t var_a = encode_var('a', ' ');

  ASSERT(is_known[var_a], "Execution ended but value of A is not known!");
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", known_val[var_a]);
}

static void solution2(const char *const input, char *const output) {
  solution(input);

  // Keep value of a
  const uint16_t var_a = encode_var('a', ' ');
  ASSERT(is_known[var_a], "First execution ended but value of A is not known!");
  const uint16_t val_a = known_val[var_a];

  // Reset state
  memset(known_val, 0, sizeof(known_val));
  memset(is_known, 0, sizeof(is_known));
  memset(needs, 0, sizeof(needs));
  memset(unknown, 0, sizeof(unknown));

  // Override value of b with value of a
  const uint16_t var_b = encode_var('b', ' ');
  known_val[var_b] = val_a;
  is_known[var_b] = true;

  solution(input);

  // Final value of a
  ASSERT(is_known[var_a], "Second execution ended but value of A is not known!");
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", known_val[var_a]);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
