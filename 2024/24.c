#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define WIRES_DIM (('z' - 'a' + 1) + ('9' - '0' + 1))
#define WIRES_LEN (WIRES_DIM * WIRES_DIM * WIRES_DIM)
#define CHAR2NUM(c) (((c) <= '9') ? ((c) - '0') : ((c) - 'a' + 10))
#define NUM2CHAR(n) ((n) < 10) ? ((n) + '0') : ((n)-10 + 'a')
#define WIRES_IDX(name)                                                                                                \
  (CHAR2NUM((name)[0]) * WIRES_DIM * WIRES_DIM + CHAR2NUM((name)[1]) * WIRES_DIM + CHAR2NUM((name)[2]))
#define WIRES_NAME1(idx) NUM2CHAR((idx) / WIRES_DIM / WIRES_DIM)
#define WIRES_NAME2(idx) NUM2CHAR(((idx) / WIRES_DIM) % WIRES_DIM)
#define WIRES_NAME3(idx) NUM2CHAR((idx) % WIRES_DIM)
#define WIRES_NAME(idx, s)                                                                                             \
  do {                                                                                                                 \
    s[0] = WIRES_NAME1(idx);                                                                                           \
    s[1] = WIRES_NAME2(idx);                                                                                           \
    s[2] = WIRES_NAME3(idx);                                                                                           \
  } while (0)

enum wire {
  WIRE_UNKNOWN,
  WIRE_LOW,
  WIRE_HIGH,
};

enum gate_type {
  GATE_NONE,
  GATE_AND,
  GATE_OR,
  GATE_XOR,
};

struct gate {
  int input1;
  int input2;
  int output;
  enum wire output_value;
  enum gate_type gate;
};

static void parse_input(const char *input, struct gate *wires) {
  const char *s;

  while (*input != '\n') {
    int n = WIRES_IDX(input);
    if (input[0] == 'z') {
    }
    input += 3;
    s = ": ";
    aoc_expect_text(&input, s, strlen(s));
    if (*input == '1') {
      wires[n].output_value = WIRE_HIGH;
    } else if (*input == '0') {
      wires[n].output_value = WIRE_LOW;
    } else {
      FAIL("parse error");
    }
    input += 2;
  }

  input += 1;

  while (*input != '\0') {
    int a = WIRES_IDX(input);

    input += 3;
    aoc_expect_char(&input, ' ');

    enum gate_type g;
    if (*input == 'X') {
      s = "XOR";
      aoc_expect_text(&input, s, strlen(s));
      g = GATE_XOR;
    } else if (*input == 'O') {
      s = "OR";
      aoc_expect_text(&input, s, strlen(s));
      g = GATE_OR;
    } else if (*input == 'A') {
      s = "AND";
      aoc_expect_text(&input, s, strlen(s));
      g = GATE_AND;
    } else {
      FAIL("parse error");
    }
    input += 1;

    int b = WIRES_IDX(input);
    input += 3;

    s = " -> ";
    aoc_expect_text(&input, s, strlen(s));

    int c = WIRES_IDX(input);
    input += 3;

    if (*input == '\n') {
      input++;
    }

    wires[c].input1 = a;
    wires[c].input2 = b;
    wires[c].output = c;
    wires[c].gate = g;
  }
}

static void evaluate_wire(int idx, struct gate *wires) {
  struct gate *wire = &wires[idx];
  if (wire->output_value != WIRE_UNKNOWN) {
    return;
  }

  ASSERT(wire->input1 != idx && wire->input2 != idx, "infinite loop, bug!!");

  evaluate_wire(wire->input1, wires);
  evaluate_wire(wire->input2, wires);
  int a = (int)(wires[wire->input1].output_value) - 1;
  int b = (int)(wires[wire->input2].output_value) - 1;
  int c;
  switch (wire->gate) {
  case GATE_XOR:
    c = a ^ b;
    break;
  case GATE_OR:
    c = a | b;
    break;
  case GATE_AND:
    c = a & b;
    break;
  default:
    FAIL("invalid gate");
  }

  if (c) {
    wire->output_value = WIRE_HIGH;
  } else {
    wire->output_value = WIRE_LOW;
  }
}

static unsigned long evaluate_all(struct gate *wires) {
  unsigned long res = 0;
  for (int i = 0;; i++) {
    char name[3];
    name[0] = 'z';
    name[1] = (i / 10) % 10 + '0';
    name[2] = i % 10 + '0';

    int idx = WIRES_IDX(name);
    if (wires[idx].output_value == WIRE_UNKNOWN && wires[idx].gate == GATE_NONE) {
      break;
    }

    evaluate_wire(idx, wires);
    if (wires[idx].output_value == WIRE_HIGH) {
      res += 1L << i;
    }
  }
  return res;
}

static void solution1(const char *input, char *const output) {
  static struct gate wires[WIRES_LEN];
  parse_input(input, wires);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", evaluate_all(wires));
}

static int find_gate(struct aoc_dynarr *gates_by_input, int input1, int input2, enum gate_type gate_type, int *swaps,
                     int *currswap) {
  char str1[3];
  char str2[3];
  char str3[3];
  const char *strgate;
  WIRES_NAME(input1, str1);
  WIRES_NAME(input2, str2);
  strgate = (char *[]){"<UNK>", "AND", "OR", "XOR"}[gate_type];
  DBG("Looking for %.3s %s %.3s", str1, strgate, str2);

  const struct aoc_dynarr *arr = &gates_by_input[input1];
  for (int i = 0; i < arr->len; i++) {
    struct gate *gate = AOC_DYNARR_IDX(*arr, i, struct gate *);
    if (gate->gate == gate_type) {
      if (gate->input1 == input1) {
        if (gate->input2 != input2) {
          WIRES_NAME(gate->input2, str3);
          DBG("!!!!!!!!!!  must replace %.3s with %.3s", str2, str3);
          swaps[(*currswap)++] = input2;
          swaps[(*currswap)++] = gate->input2;
        }
        return gate->output;
      }
      if (gate->input2 == input1) {
        if (gate->input1 != input2) {
          WIRES_NAME(gate->input1, str3);
          DBG("!!!!!!!!!!  must replace %.3s with %.3s", str2, str3);
          swaps[(*currswap)++] = input2;
          swaps[(*currswap)++] = gate->input1;
        }
        return gate->output;
      }
    }
  }

  arr = &gates_by_input[input2];
  for (int i = 0; i < arr->len; i++) {
    struct gate *gate = AOC_DYNARR_IDX(*arr, i, struct gate *);
    if (gate->gate == gate_type) {
      if (gate->input1 == input2) {
        if (gate->input2 != input1) {
          WIRES_NAME(gate->input2, str3);
          DBG("!!!!!!!!!!  must replace %.3s with %.3s", str1, str3);
          swaps[(*currswap)++] = input1;
          swaps[(*currswap)++] = gate->input2;
        }
        return gate->output;
      }
      if (gate->input2 == input2) {
        if (gate->input1 != input1) {
          WIRES_NAME(gate->input1, str3);
          DBG("!!!!!!!!!!  must replace %.3s with %.3s", str1, str3);
          swaps[(*currswap)++] = input1;
          swaps[(*currswap)++] = gate->input1;
        }
        return gate->output;
      }
    }
  }

  FAIL("could not find gate");
}

static void solution2(const char *input, char *const output) {
  static struct gate gates[WIRES_LEN];
  parse_input(input, gates);

  static struct aoc_dynarr gates_by_input[WIRES_LEN];
  for (int i = 0; i < WIRES_LEN; i++) {
    if (gates[i].gate == GATE_NONE) {
      continue;
    }
    struct aoc_dynarr *arr1 = &gates_by_input[gates[i].input1];
    struct aoc_dynarr *arr2 = &gates_by_input[gates[i].input2];
    if (arr1->size == 0) {
      aoc_dynarr_init(arr1, sizeof(struct gate *), 4);
    }
    if (arr2->size == 0) {
      aoc_dynarr_init(arr2, sizeof(struct gate *), 4);
    }
    struct gate **g = aoc_dynarr_grow(arr1, 1);
    *g = &gates[i];
    g = aoc_dynarr_grow(arr2, 1);
    *g = &gates[i];
  }

  // Alright this is just a bunch of full adders.
  // We expect:
  //   x1_n = x_n  XOR y_n
  //   x2_n = x_n  AND y_n
  //   x3_n = x1_n AND c_(n-1)
  //   z_n  = x1_n XOR c_(n-1)
  //   c_n  = x2_n OR  x3_n
  // Except for the last z_n where
  //   z_45 = c_44
  // And for the first n where
  //   z_0  = x_0  XOR y_0
  //   c_0  = x_0  AND y_0

  int swaps[64];
  int currswap = 0;

  char str[3];

  int c_prev = find_gate(gates_by_input, WIRES_IDX("x00"), WIRES_IDX("y00"), GATE_AND, swaps, &currswap);
  WIRES_NAME(c_prev, str);
  DBG("found c_0: %.*s", 3, str);

  for (int i = 1; i <= 44; i++) {
    DBG("bit %d", i);

    str[1] = i / 10 + '0';
    str[2] = i % 10 + '0';

    str[0] = 'x';
    int x = WIRES_IDX(str);
    str[0] = 'y';
    int y = WIRES_IDX(str);
    str[0] = 'z';
    int z = WIRES_IDX(str);

    int x1 = find_gate(gates_by_input, x, y, GATE_XOR, swaps, &currswap);
    WIRES_NAME(x1, str);
    DBG("found x1_%d: %.*s", i, 3, str);

    int x2 = find_gate(gates_by_input, x, y, GATE_AND, swaps, &currswap);
    WIRES_NAME(x2, str);
    DBG("found x2_%d: %.*s", i, 3, str);

    int x3 = find_gate(gates_by_input, x1, c_prev, GATE_AND, swaps, &currswap);
    WIRES_NAME(x3, str);
    DBG("found x3_%d: %.*s", i, 3, str);

    int zz = find_gate(gates_by_input, x1, c_prev, GATE_XOR, swaps, &currswap);
    WIRES_NAME(zz, str);
    if (zz != z) {
      swaps[currswap] = zz;
      swaps[currswap + 1] = z;
      currswap += 2;
      WIRES_NAME(z, str);
      char str2[3];
      WIRES_NAME(zz, str2);
      DBG("!!!!!!!!!!  must replace %.3s with %.3s", str2, str);
    }
    DBG("verified z_%d: %.*s", i, 3, str);

    int c = find_gate(gates_by_input, x2, x3, GATE_OR, swaps, &currswap);
    WIRES_NAME(c, str);
    DBG("found c_%d: %.*s", i, 3, str);

    c_prev = c;
  }

  qsort(swaps, currswap, sizeof(swaps[0]), aoc_cmp_int);

  char res[32];

  int prev = 0;
  char *r = res;
  for (int i = 0; i < currswap; i++) {
    if (swaps[i] == prev) {
      continue;
    }
    prev = swaps[i];

    WIRES_NAME(swaps[i], r);
    r += 3;
    *r = ',';
    r++;
  }
  r--;
  *r = '\0';

  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", res);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
