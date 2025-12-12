#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct button {
  int npos;
  int *pos;
};

struct machine {
  int nlights;
  bool *lights;

  int nbuttons;
  struct button *buttons;

  int njolts;
  int *jolts;
};

static int *get_coefficient_matrix(const struct machine *machine, int *n, int *m) {
  *n = machine->nbuttons;
  *m = machine->nlights;

  int *res = malloc(sizeof(*res) * *n * *m);
  memset(res, 0, sizeof(*res) * *n * *m);

  for (int i = 0; i < machine->nbuttons; i++) {
    for (int j = 0; j < machine->buttons[i].npos; j++) {
      res[AOC_2D_IDX(i, machine->buttons[i].pos[j], *n)] = 1;
    }
  }

  return res;
}

static int *get_lights_vector(const struct machine *machine) {
  int *res = malloc(sizeof(*res) * machine->nlights);

  for (int j = 0; j < machine->nlights; j++) {
    if (machine->lights[j]) {
      res[j] = 1;
    } else {
      res[j] = 0;
    }
  }

  return res;
}

static int *get_jolts_vector(const struct machine *machine) {
  int *res = malloc(sizeof(*res) * machine->njolts);

  for (int j = 0; j < machine->njolts; j++) {
    res[j] = machine->jolts[j];
  }

  return res;
}

#ifdef DEBUG
static void print_machine(const struct machine *machine) {
  fputc('[', stderr);
  for (int i = 0; i < machine->nlights; i++) {
    if (machine->lights[i]) {
      fputc('#', stderr);
    } else {
      fputc('.', stderr);
    }
  }
  fputc(']', stderr);

  fputc(' ', stderr);

  for (int i = 0; i < machine->nbuttons; i++) {
    fputc('(', stderr);
    for (int j = 0; j < machine->buttons[i].npos; j++) {
      fprintf(stderr, "%d", machine->buttons[i].pos[j]);
      if (j < machine->buttons[i].npos - 1) {
        fputc(',', stderr);
      }
    }
    fputc(')', stderr);

    if (i < machine->nbuttons - 1) {
      fputc(' ', stderr);
    }
  }

  // yeah, i know, why condition the last space on the sequence of buttons if i need one after anyway
  // shut up
  fputc(' ', stderr);

  fputc('{', stderr);
  for (int i = 0; i < machine->njolts; i++) {
    fprintf(stderr, "%d", machine->jolts[i]);
    if (i < machine->njolts - 1) {
      fputc(',', stderr);
    }
  }
  fputc('}', stderr);
}
#endif

static bool parse_number(const char **input, void *ptr) {
  if (!isdigit(**input)) {
    return false;
  }

  int *n = ptr;
  *n = aoc_parse_int(input);
  if (**input == ',') {
    *input += 1;
    aoc_skip_space(input);
  }

  return true;
}

static bool parse_button(const char **input, void *ptr) {
  if (**input == '{') {
    return false;
  }

  struct button *button = ptr;
  aoc_expect_char(input, '(');
  button->pos = aoc_parse_sequence(input, &button->npos, sizeof(*button->pos), 4, parse_number);
  aoc_expect_char(input, ')');
  aoc_skip_space(input);
  return true;
}

static bool parse_light(const char **input, void *ptr) {
  if (**input == ']') {
    return false;
  }

  bool *light = ptr;
  if (**input == '.') {
    *light = false;
  } else if (**input == '#') {
    *light = true;
  } else {
    FAIL("parse error");
  }
  *input += 1;

  return true;
}

static bool parse_machine(const char **input, void *ptr) {
  if (**input == '\0') {
    return false;
  }

  struct machine *machine = ptr;
  aoc_expect_char(input, '[');
  machine->lights = aoc_parse_sequence(input, &machine->nlights, sizeof(*machine->lights), 8, parse_light);
  aoc_expect_char(input, ']');

  aoc_skip_space(input);

  machine->buttons = aoc_parse_sequence(input, &machine->nbuttons, sizeof(*machine->buttons), 8, parse_button);

  aoc_expect_char(input, '{');
  machine->jolts = aoc_parse_sequence(input, &machine->njolts, sizeof(*machine->jolts), 8, parse_number);
  aoc_expect_char(input, '}');

  aoc_skip_space(input);

  ASSERT(machine->njolts == machine->nlights, "parse error");
  return true;
}

#define SWAP(a, b, type)                                                                                               \
  do {                                                                                                                 \
    type _tmp = a;                                                                                                     \
    a = b;                                                                                                             \
    b = _tmp;                                                                                                          \
  } while (0)

struct rational {
  long num, den;
};

__attribute__((const)) static long get_gcd(long a, long b) {
  while (b != 0) {
    long t = b;
    b = a % b;
    a = t;
  }
  return a;
}

__attribute__((const)) static struct rational rational_simplify(struct rational r) {
  long gcd = get_gcd(r.num, r.den);

  r.num /= gcd;
  r.den /= gcd;
  if (r.den < 0) {
    r.num = -r.num;
    r.den = -r.den;
  }

  return r;
}

static struct rational rational_init(long n) {
  struct rational r = {
      .num = n,
      .den = 1,
  };
  return r;
}

__attribute__((const)) static struct rational rational_sum(struct rational r1, struct rational r2) {
  struct rational res;
  res.num = r1.num * r2.den + r2.num * r1.den;
  res.den = r1.den * r2.den;

  return rational_simplify(res);
}

__attribute__((const)) static struct rational rational_sub(struct rational r1, struct rational r2) {
  r2.num = -r2.num;
  return rational_sum(r1, r2);
}

__attribute__((const)) static struct rational rational_mul(struct rational r1, struct rational r2) {
  struct rational res;
  res.num = r1.num * r2.num;
  res.den = r1.den * r2.den;
  return rational_simplify(res);
}

static struct rational rational_inv(struct rational r) {
  ASSERT(r.num != 0, "division by zero");
  long n = r.num;
  r.num = r.den;
  r.den = n;
  return r;
}

static bool rational_is_zero(struct rational r) { return r.num == 0; }

static int rational_is_integer(struct rational r) { return (r.den == 1 || r.den == -1); }

static int rational_is_nonnegative(struct rational r) { return r.num >= 0; }

__attribute__((const)) static int rational_cmp(struct rational a, struct rational b) {
  struct rational diff = rational_sub(a, b);
  if (diff.num < 0)
    return -1;
  if (diff.num > 0)
    return 1;
  return 0;
}

#ifdef DEBUG
static void rational_print(FILE *stream, struct rational r) { fprintf(stream, "%ld/%ld", r.num, r.den); }
#endif

struct rational_linear_system_solution {
  int n;                      // number of columns
  int k;                      // number of free variables
  struct rational *solution;  // one particular solution (len = n)
  struct rational *nullspace; // nullspace basis (AOC_2D_IDX(col, vec, n), n entries per vec, k vecs)
};

static struct rational_linear_system_solution solve_q_inner(struct rational *A, struct rational *b, int n, int m) {
  int row = 0;

  int *pivot_positions = malloc(sizeof(*pivot_positions) * n);
  for (int i = 0; i < n; i++) {
    pivot_positions[i] = -1;
  }

  // forward elimination (to row echelon form, then normalize pivots)
  for (int col = 0; col < n && row < m; col++) {
    // find a row with nonzero in this column at or below current row
    int pivot = -1;
    for (int r = row; r < m; r++) {
      if (!rational_is_zero(A[AOC_2D_IDX(col, r, n)])) {
        pivot = r;
        break;
      }
    }

    // no pivot in this column
    if (pivot == -1) {
      continue;
    }

    // swap pivot row up
    for (int i = 0; i < n; i++) {
      SWAP(A[AOC_2D_IDX(i, pivot, n)], A[AOC_2D_IDX(i, row, n)], struct rational);
    }
    SWAP(b[pivot], b[row], struct rational);

    pivot_positions[col] = row;

    // normalize pivot row so A[col, row] becomes 1
    struct rational pivot_val = A[AOC_2D_IDX(col, row, n)];
    struct rational inv_pivot = rational_inv(pivot_val);
    for (int c = col; c < n; c++) {
      A[AOC_2D_IDX(c, row, n)] = rational_mul(A[AOC_2D_IDX(c, row, n)], inv_pivot);
    }
    b[row] = rational_mul(b[row], inv_pivot);

    // eliminate entries below pivot: r <- r - factor * pivot_row
    for (int r = row + 1; r < m; r++) {
      struct rational factor = A[AOC_2D_IDX(col, r, n)];
      if (rational_is_zero(factor)) {
        continue;
      }
      for (int c = col; c < n; c++) {
        struct rational tmp = rational_mul(factor, A[AOC_2D_IDX(c, row, n)]);
        A[AOC_2D_IDX(c, r, n)] = rational_sub(A[AOC_2D_IDX(c, r, n)], tmp);
      }
      struct rational tmpb = rational_mul(factor, b[row]);
      b[r] = rational_sub(b[r], tmpb);
    }

    row += 1;
  }

  // backward elimination (to reduced row echelon form)
  for (int col = n - 1; col >= 0; col--) {
    int r = pivot_positions[col];
    if (r == -1) {
      continue;
    }

    // eliminate entries above pivot: r2 <- r2 - factor * pivot_row
    for (int r2 = 0; r2 < r; r2++) {
      struct rational factor = A[AOC_2D_IDX(col, r2, n)];
      if (rational_is_zero(factor)) {
        continue;
      }
      for (int c = col; c < n; c++) {
        struct rational tmp = rational_mul(factor, A[AOC_2D_IDX(c, r, n)]);
        A[AOC_2D_IDX(c, r2, n)] = rational_sub(A[AOC_2D_IDX(c, r2, n)], tmp);
      }
      struct rational tmpb = rational_mul(factor, b[r]);
      b[r2] = rational_sub(b[r2], tmpb);
    }
  }

  // detect inconsistency: 0 ... 0 | b[r] ≠ 0
  for (int r = 0; r < m; r++) {
    int all_zero = 1;
    for (int c = 0; c < n; c++) {
      if (!rational_is_zero(A[AOC_2D_IDX(c, r, n)])) {
        all_zero = 0;
        break;
      }
    }

    if (!all_zero) {
      continue;
    }

    if (!rational_is_zero(b[r])) {
      ASSERT(0, "unsolvable system");
    }
  }

  struct rational_linear_system_solution sol;
  sol.n = n;

  // construct particular solution (set all free vars = 0)
  sol.solution = malloc(sizeof(*sol.solution) * n);
  for (int i = 0; i < n; i++) {
    sol.solution[i] = rational_init(0);
  }

  // assign pivot variables from b
  for (int col = 0; col < n; col++) {
    int r = pivot_positions[col];
    if (r != -1) {
      sol.solution[col] = b[r];
    }
  }

  // collect free variables
  int *free_vars = malloc(sizeof(*free_vars) * n);
  sol.k = 0;
  for (int col = 0; col < n; col++) {
    if (pivot_positions[col] == -1) {
      free_vars[sol.k++] = col;
    }
  }

  // build nullspace basis
  sol.nullspace = malloc(sizeof(*sol.nullspace) * n * sol.k);

  // initialize to 0
  for (int j = 0; j < sol.k; j++) {
    for (int col = 0; col < n; col++) {
      sol.nullspace[AOC_2D_IDX(col, j, n)] = rational_init(0);
    }
  }

  // For each free variable f_j, create one basis vector:
  //   set x_fj = 1, other free vars = 0,
  //   and solve for pivot variables:
  //     x_pivot = - A[fj, row_of_pivot]  (because Ax = 0)
  for (int j = 0; j < sol.k; j++) {
    int f = free_vars[j];

    // free variable itself = 1
    sol.nullspace[AOC_2D_IDX(f, j, n)] = rational_init(1);

    // pivot variables
    for (int p = 0; p < n; p++) {
      int r = pivot_positions[p];
      if (r == -1) {
        continue;
      }

      struct rational coeff = A[AOC_2D_IDX(f, r, n)];

      // x_pivot = -coeff
      struct rational zero = rational_init(0);
      sol.nullspace[AOC_2D_IDX(p, j, n)] = rational_sub(zero, coeff);
    }
  }

  free(pivot_positions);
  free(free_vars);
  return sol;
}

static struct rational_linear_system_solution solve_q(int *A, int *b, int n, int m) {
  struct rational *rA = malloc(sizeof(*rA) * n * m);
  struct rational *rb = malloc(sizeof(*rb) * m);

  for (int j = 0; j < m; j++) {
    rb[j] = rational_init(b[j]);
    for (int i = 0; i < n; i++) {
      int idx = AOC_2D_IDX(i, j, n);
      rA[idx] = rational_init(A[idx]);
    }
  }

  struct rational_linear_system_solution sol = solve_q_inner(rA, rb, n, m);
  free(rA);
  free(rb);

  return sol;
}

struct gf2_linear_system_solution {
  int n;          // n passed in (num columns)
  int k;          // number of free variables
  int *solution;  // one solution (len=n)
  int *nullspace; // nullspace basis (AOC_2D_IDX(col, vec, n) for indexing, n values per vec, k vecs)
};

static struct gf2_linear_system_solution solve_gf2(int *A, int *b, int n, int m) {
  int row = 0;

  int *pivot_positions = malloc(sizeof(*pivot_positions) * n);
  for (int i = 0; i < n; i++) {
    pivot_positions[i] = -1;
  }

  // forward elimination
  for (int col = 0; col < n; col++) {
    // find a row with 1 in this column at or below current row
    int pivot = -1;
    for (int r = row; r < m; r++) {
      if (A[AOC_2D_IDX(col, r, n)] == 1) {
        pivot = r;
        break;
      }
    }

    // no pivot in this column
    if (pivot == -1) {
      continue;
    }

    // swap pivot row up
    for (int i = 0; i < n; i++) {
      SWAP(A[AOC_2D_IDX(i, pivot, n)], A[AOC_2D_IDX(i, row, n)], int);
    }
    SWAP(b[pivot], b[row], int);

    pivot_positions[col] = row;

    // eliminate entries below pivot
    for (int r = row + 1; r < m; r++) {
      if (A[AOC_2D_IDX(col, r, n)] == 1) {
        // r <- r xor row
        for (int c = 0; c < n; c++) {
          A[AOC_2D_IDX(c, r, n)] ^= A[AOC_2D_IDX(c, row, n)];
        }
        b[r] = b[r] ^ b[row];
      }
    }

    row += 1;
    if (row == m) {
      break;
    }
  }

  // backward elimination
  for (int col = n - 1; col >= 0; col--) {
    int r = pivot_positions[col];
    if (r == -1) {
      continue;
    }

    // eliminate entries above pivot
    for (int r2 = 0; r2 < r; r2++) {
      if (A[AOC_2D_IDX(col, r2, n)] == 1) {
        for (int c = 0; c < n; c++) {
          A[AOC_2D_IDX(c, r2, n)] ^= A[AOC_2D_IDX(c, r, n)];
        }
        b[r2] = b[r2] ^ b[r];
      }
    }
  }

  // Determine unsolvable system
  for (int r = 0; r < m; r++) {
    if (b[r] == 0) {
      continue;
    }

    bool ok = false;
    for (int c = 0; c < n; c++) {
      if (A[AOC_2D_IDX(c, r, n)] == 1) {
        ok = true;
        break;
      }
    }

    ASSERT(ok, "unsolvable system");
  }

  struct gf2_linear_system_solution sol;
  sol.n = n;

  // construct particular solution
  sol.solution = malloc(sizeof(*sol.solution) * n);
  for (int i = 0; i < n; i++) {
    sol.solution[i] = 0;
  }

  // assign pivot variables from b
  for (int col = 0; col < n; col++) {
    int r = pivot_positions[col];
    if (r != -1) {
      sol.solution[col] = b[r];
    }
  }

  // collect free variables
  int *free_vars = malloc(sizeof(*free_vars) * n);
  sol.k = 0;

  for (int col = 0; col < n; col++) {
    if (pivot_positions[col] == -1) {
      free_vars[sol.k++] = col;
    }
  }

  // build nullspace vector
  sol.nullspace = malloc(sizeof(*sol.nullspace) * n * sol.k);

  for (int r = 0; r < sol.k; r++) {
    for (int col = 0; col < n; col++) {
      sol.nullspace[AOC_2D_IDX(col, r, n)] = 0;
    }
  }

  for (int j = 0; j < sol.k; j++) {
    int f = free_vars[j];
    sol.nullspace[AOC_2D_IDX(f, j, n)] = 1;
    for (int p = 0; p < n; p++) {
      int r = pivot_positions[p];
      if (r == -1) {
        continue;
      }

      sol.nullspace[AOC_2D_IDX(p, j, n)] = A[AOC_2D_IDX(f, r, n)];
    }
  }

  free(pivot_positions);
  free(free_vars);
  return sol;
}

static int *best_solution_gf2(struct gf2_linear_system_solution sol) {
  int best_weight = sol.n;
  int *best = malloc(sizeof(*best) * sol.n);
  int *candidate = malloc(sizeof(*candidate) * sol.n);

  for (int mask = 0; mask < (1 << sol.k); mask++) {
    for (int i = 0; i < sol.n; i++) {
      candidate[i] = sol.solution[i];
    }

    for (int j = 0; j < sol.k; j++) {
      if (mask & (1 << j)) {
        for (int i = 0; i < sol.n; i++) {
          candidate[i] ^= sol.nullspace[AOC_2D_IDX(i, j, sol.n)];
        }
      }
    }

    int w = 0;
    for (int i = 0; i < sol.n; i++) {
      w += candidate[i];
    }

    if (w < best_weight) {
      best_weight = w;
      memcpy(best, candidate, sizeof(*best) * sol.n);

#ifdef DEBUG
      fprintf(stderr, "best =\n");
      for (int i = 0; i < sol.n; i++) {
        fprintf(stderr, "%d", best[i]);
        if (i < sol.n - 1) {
          fputc(',', stderr);
        }
      }
      fputc('\n', stderr);
#endif
    }
  }

#ifdef DEBUG
  fputc('\n', stderr);
#endif

  free(candidate);
  return best;
}

/*** Recursive search over integer parameters for the nullspace ***/

static void search_params_recursive(const struct rational_linear_system_solution *sol, int max_param, int idx,
                                    int *params,
                                    struct rational *candidate,      // temp buffer length sol->n
                                    struct rational *best_sum,       // single scalar
                                    struct rational *best_candidate, // best candidate vector length sol->n
                                    int *found                       // boolean flag
) {
  int n = sol->n;
  int k = sol->k;

  if (idx == k) {
    // Build candidate = particular_solution + sum_j params[j] * nullspace_j
    for (int i = 0; i < n; i++) {
      candidate[i] = sol->solution[i];
    }

    for (int j = 0; j < k; j++) {
      if (params[j] == 0)
        continue;
      struct rational pj = rational_init(params[j]);
      for (int i = 0; i < n; i++) {
        struct rational term = rational_mul(pj, sol->nullspace[AOC_2D_IDX(i, j, n)]);
        candidate[i] = rational_sum(candidate[i], term);
      }
    }

    // Check integrality and non-negativity
    for (int i = 0; i < n; i++) {
      if (!rational_is_integer(candidate[i])) {
        return; // not an integer solution
      }
      if (!rational_is_nonnegative(candidate[i])) {
        return; // violates x >= 0
      }
    }

    // Compute sum(x_i)
    struct rational sum = rational_init(0);
    for (int i = 0; i < n; i++) {
      sum = rational_sum(sum, candidate[i]);
    }

    // Update best if needed
    if (!*found) {
      *found = 1;
      *best_sum = sum;
      memcpy(best_candidate, candidate, sizeof(*candidate) * n);
    } else {
      if (rational_cmp(sum, *best_sum) < 0) {
        *best_sum = sum;
        memcpy(best_candidate, candidate, sizeof(*candidate) * n);
      }
    }

    return;
  }

  // Enumerate params[idx] in [-max_param, max_param]
  for (int v = -max_param; v <= max_param; v++) {
    params[idx] = v;
    search_params_recursive(sol, max_param, idx + 1, params, candidate, best_sum, best_candidate, found);
  }
}

/*** Public function: best nonnegative integer solution with minimal sum ***/

// Returns an array of length sol.n with the chosen x[i] as long integers.
// Asserts if no solution is found within the search bounds.
static int *best_solution_q(struct rational_linear_system_solution sol, int max_param_abs) {
  int n = sol.n;
  int k = sol.k;

  // Trivial case: no free variables, just check the particular solution.
  if (k == 0) {
    DBG("trivial: no free variables");
    for (int i = 0; i < n; i++) {
      ASSERT(rational_is_integer(sol.solution[i]), "particular solution is not integer");
      ASSERT(rational_is_nonnegative(sol.solution[i]), "particular solution is not nonnegative");
    }
    int *best_int = malloc(sizeof(*best_int) * n);
    for (int i = 0; i < n; i++) {
      struct rational r = rational_simplify(sol.solution[i]);
      if (r.den < 0) {
        r.den = -r.den;
        r.num = -r.num;
      }
      best_int[i] = r.num / r.den;
    }
    return best_int;
  }

  int *params = malloc(sizeof(*params) * k);
  struct rational *candidate = malloc(sizeof(*candidate) * n);
  struct rational *best_candidate = malloc(sizeof(*best_candidate) * n);

  struct rational best_sum = rational_init(0);
  int found = 0;

  search_params_recursive(&sol, max_param_abs, 0, params, candidate, &best_sum, best_candidate, &found);

  free(params);
  free(candidate);

  if (!found) {
    DBG("no nonnegative integer solution within parameter bounds: %d", max_param_abs);
    free(best_candidate);
    return NULL;
  }

  // Convert best_candidate (rationals) to integer array
  int *best_int = malloc(sizeof(*best_int) * n);
  for (int i = 0; i < n; i++) {
    struct rational r = rational_simplify(best_candidate[i]);
    if (r.den < 0) {
      r.den = -r.den;
      r.num = -r.num;
    }
    // at this point r should be an integer and >= 0
    ASSERT(r.den == 1, "best solution entry not integer after simplify");
    best_int[i] = r.num;
  }

  free(best_candidate);
  return best_int;
}

static void solution(const char *input, char *const output, bool jolts) {
  int nmachines;
  struct machine *machines = aoc_parse_sequence(&input, &nmachines, sizeof(*machines), 16, parse_machine);

  int total = 0;
  for (int i = 0; i < nmachines; i++) {
    int n, m;
    int *A = get_coefficient_matrix(&machines[i], &n, &m);
    int *b;
    if (jolts) {
      b = get_jolts_vector(&machines[i]);
    } else {
      b = get_lights_vector(&machines[i]);
    }

#ifdef DEBUG
    print_machine(&machines[i]);
    fputc('\n', stderr);

    for (int j = 0; j < m; j++) {
      fputc('[', stderr);
      for (int ii = 0; ii < n; ii++) {
        fprintf(stderr, "%d", A[AOC_2D_IDX(ii, j, n)]);
      }
      fputc('|', stderr);
      fprintf(stderr, "%d", b[j]);
      fputc(']', stderr);
      fputc('\n', stderr);
    }
    fputc('\n', stderr);
#endif

    int *best;
    if (jolts) {
      struct rational_linear_system_solution sol = solve_q(A, b, n, m);
#ifdef DEBUG
      fprintf(stderr, "k = %d\n", sol.k);
      fprintf(stderr, "solution =\n");
      for (int ii = 0; ii < sol.n; ii++) {
        rational_print(stderr, sol.solution[ii]);
        if (ii < sol.n - 1) {
          fputc(',', stderr);
        }
      }
      fputc('\n', stderr);

      fprintf(stderr, "nullspace =\n");
      for (int j = 0; j < sol.k; j++) {
        for (int ii = 0; ii < sol.n; ii++) {
          rational_print(stderr, sol.nullspace[AOC_2D_IDX(ii, j, n)]);
          if (ii < sol.n - 1) {
            fputc(',', stderr);
          }
        }
        if (j < sol.k - 1) {
          fputc('\n', stderr);
        }
      }
      fputc('\n', stderr);
      fputc('\n', stderr);
#endif

      int bounds = 40;
      best = NULL;
      while (best == NULL) {
        best = best_solution_q(sol, bounds);
        bounds += 10;
      }
      free(sol.nullspace);
      free(sol.solution);
    } else {
      struct gf2_linear_system_solution sol = solve_gf2(A, b, n, m);
#ifdef DEBUG
      fprintf(stderr, "k = %d\n", sol.k);
      fprintf(stderr, "solution =\n");
      for (int ii = 0; ii < sol.n; ii++) {
        fprintf(stderr, "%d", sol.solution[ii]);
        if (ii < sol.n - 1) {
          fputc(',', stderr);
        }
      }
      fputc('\n', stderr);

      fprintf(stderr, "nullspace =\n");
      for (int j = 0; j < sol.k; j++) {
        for (int ii = 0; ii < sol.n; ii++) {
          fprintf(stderr, "%d", sol.nullspace[AOC_2D_IDX(ii, j, n)]);
          if (ii < sol.n - 1) {
            fputc(',', stderr);
          }
        }
        if (j < sol.k - 1) {
          fputc('\n', stderr);
        }
      }
      fputc('\n', stderr);
      fputc('\n', stderr);
#endif

      best = best_solution_gf2(sol);
      free(sol.nullspace);
      free(sol.solution);
    }

#ifdef DEBUG
    int w = 0;
    for (int j = 0; j < n; j++) {
      w += best[j];
    }
    fprintf(stderr, "weight = %d\n", w);
#endif

    for (int j = 0; j < n; j++) {
      total += best[j];
    }

    free(A);
    free(b);
    free(best);

#ifdef DEBUG
    fprintf(stderr, "\n\n----------\n\n");
#endif
  }

  for (int i = 0; i < nmachines; i++) {
    free(machines[i].lights);
    for (int j = 0; j < machines[i].nbuttons; j++) {
      free(machines[i].buttons[j].pos);
    }
    free(machines[i].buttons);
    free(machines[i].jolts);
  }
  free(machines);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
