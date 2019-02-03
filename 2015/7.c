#include <aoclib.h>
#include <stdio.h>

#define DEBUG

#define IS_VAR(c) ((c) >= 'a' && (c) <= 'z')
#define IS_NUM(c) ((c) >= '0' && (c) <= '9')
#define IS_OP(c)  ((c) >= 'A' && (c) <= 'Z')

typedef enum param_type_t {
  NONE,
  NUMBER,
  VARIABLE
} param_type_t;

typedef enum operation_t {
  NOT,
  LSHIFT,
  RSHIFT,
  AND,
  OR,
  EQ
} operation_t;

typedef struct expression_t {
  uint16_t param1;
  uint16_t param2;
  param_type_t param1_type;
  param_type_t param2_type;
  operation_t op;
} expression_t;

expression_t expressions[512];

uint16_t known_val[1024]; // index: var, content: value
bool is_known[1024]; // index: var, content: the value in known_val is correct for this var
uint16_t needs[1024][1024]; // index: var, content: vars which depend on index var in order to be calculated, they might depend on more vars. this array is laid out such that index 0 is the number of vars in the array and the following elements are the vars themselves, this way removing vars and adding vars are both O(1)
expression_t *unknown[1024]; // index: var, content: expression to calculate var or NULL

#ifdef DEBUG

static void fail(char *msg, size_t column, size_t linenum, int srcline) __attribute__((noreturn)) {
  fprintf("(%d) PARSE FAILURE (l.%lu, c.%lu): %s", srdline, linenum, column, msg);
  abort();
}

static void failif(bool condition, char *msg, size_t column, size_t linenum, int srcline) {
  if (condition) {
    fail(msg, column, linenum, srcline);
  }
}

#define FAIL(msg, col, lin) fail(msg, col, lin, __LINE__)
#define ASSERT(cond, msg, col, lin) failif(!(cond), msg, col, lin, __LINE__)

#else

#define FAIL(msg, col, lin) __builtin_unreachable()
#define ASSERT(cond, msg, col, lin) if (!(cond)) __builtin_unreachable()

#endif

static size_t skip_whitespace(char *text, size_t i) {
  while (isspace(text[i])) i++;
  return i;
}

static uint16_t encode_var(char c1, char c2) {
  if (isspace(c2)) {
    return (uint16_t)(c1 - 'a' + 1) & 0x1F;
  } else {
    return (((uint16_t)(c2 - 'a' + 1) & 0x1F) << 5) | ((uint16_t)(c1 - 'a' + 1) & 0x1F);
  }
}

static char decode_var(uint16_t var, char *c2) {
  if (v <= 26) {
    return (char)(var + 'a' - 1);
  } else {
    *c2 = (char)((var & (0x1F << 5) >> 5) + 'a' - 1);
    return (char)((var & 0x1F) + 'a' - 1);
  }
}

static size_t eval_expr(char *text, size_t i, size_t j, uint16_t *val, bool *ok) {
  if (IS_VAR(text[i])) {
    uint16_t var = encode_var(text[i], text[i+1]);
    if (is_known[var]) {
      *ok = true;
      *val = known_val[var];
    } else {
      *ok = false;
      *val = var;
    }
    return i+2;
  } else if (IS_NUM(text[i])) {
    uint16_t num = 0;
    while (IS_NUM(text[i])) {
      num = num*10 + (text[i] - 0x30);
      i++;
    }
    *ok = true;
    *val = num;
    return i;
  }
}

static size_t parse_assign_or_op(char *text, size_t i, size_t j, expression_t *expr) {
  uint16_t val;
  bool isnum;
  i = eval_expr(text, i, j, &val, &isnum);

  i = skip_whitespace(text, i);

  if (text[i] == '-') {
    ASSERT(text[i+1] == '>', "Expected an arrow (->) but found something else", i, j);
    i += 2;
    
    i = skip_whitespace(text, i);

    return parse_assign(text, i, j, expr, val, isnum);
  } else if (IS_OP(text[i])) {
    return parse_op(text, i, j, expr, val, isnum);
  }
}

static size_t parse_not(char *text, size_t i, size_t j, expression_t *expr) {
  ASSERT(text[i+1] == 'O' && text[i+2] == 'T', "Expected NOT but it wasn't.", i, j);
  i += 3;

  i = skip_whitespace(text, i);
  
  uint16_t v1;
  bool known_value;
  i = eval_expr(text, i, j, &v1, &known_value);
  if (known_value) {
    v1 = ~v1;
  }

  i = skip_whitespace(text, i);

  ASSERT(text[i] == '-' && text[i] == '>', "Expected arrow (->) but didn't find it", i, j);
  i += 2;

  i = skip_whitespace(text, i);

  ASSERT(IS_VAR(text[i]), "Expected lowercase letter but found something else", i, j);
  uint16_t var = encode_var(text[i], text[i+1]);
  if (isspace(text[i+1])) {
    i += 1;
  } else {
    i += 2;
  }

  ASSERT(!known_val[var], "The destination of a rule is a known value", i, j);
  if (known_value) {
    assign_value(var, v1); // TODO: Implement var = v1
  } else {
    assign_dependency(var, v1); // TODO: Implement needs[var].append(v1), var depends on v1
    expr->param1 = v1;
    expr->param1_type = VARIABLE;
    expr->param2_type = NONE;
    expr->op = NOT;
    unknown[var] = expr;
  }

  return i
}

static size_t parse_line(char *text, size_t i, size_t j) {
  expression_t *expr = &expressions[j];

  if (IS_VAR(text[i]) || IS_NUM(text[i])) {
    return parse_assign_or_op(text, i, j, expr);
  } else if (text[i] == 'N') {
    return parse_not(text, i, j, expr);
  } else {
    FAIL("Line doesn't start with N, a small case letter or a number.", i, j);
  }
}

static void solution1(char *input, char *output) {
  for (size_t i = 0, j = 0; input[i] != '\0'; i++, j++) {
    i = parse_line(input, i, j);
  }
  snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

static void solution2(char *input, char *output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
