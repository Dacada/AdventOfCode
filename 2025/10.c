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

static int cmp_button(const void *ptr1, const void *ptr2) {
  const struct button *b1 = ptr1;
  const struct button *b2 = ptr2;

  return aoc_cmp_int(&b1->npos, &b2->npos);
}

struct machine {
  int nlights;
  bool *lights;

  int nbuttons;
  struct button *buttons;

  int njolts;
  int *jolts;
};

static void machine_sort(struct machine *machine) {
  qsort(machine->buttons, machine->nbuttons, sizeof(*machine->buttons), cmp_button);
}

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

static int *get_rhs_vector(const struct machine *machine) {
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

struct gf2_solution {
  int n;          // n passed in (num columns)
  int k;          // number of free variables
  int *solution;  // one solution (len=n)
  int *nullspace; // nullspace basis (AOC_2D_IDX(col, vec, n) for indexing, n values per vec, k vecs)
};

static struct gf2_solution solve_gf2(int *A, int *b, int n, int m) {
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

  struct gf2_solution sol;
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

static int *best_solution(struct gf2_solution sol) {
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

// this may have been overkill
static void solution1(const char *input, char *const output) {
  int nmachines;
  struct machine *machines = aoc_parse_sequence(&input, &nmachines, sizeof(*machines), 16, parse_machine);

  int total = 0;
  for (int i = 0; i < nmachines; i++) {
    int n, m;
    int *A = get_coefficient_matrix(&machines[i], &n, &m);
    int *b = get_rhs_vector(&machines[i]);

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

    struct gf2_solution sol = solve_gf2(A, b, n, m);
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

    int *best = best_solution(sol);

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
    free(sol.nullspace);
    free(sol.solution);

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

static int get_upper_bound(const int *A, const int *goal, int n, int m, int current, const int *counters) {
  int bound = INT_MAX;
  for (int j = 0; j < m; j++) {
    if (A[AOC_2D_IDX(current, j, n)] == 1) {
      ASSERT(goal[j] >= counters[j], "already surpased goal");
      int candidate = goal[j] - counters[j];
      if (candidate < bound) {
        bound = candidate;
      }
    }
  }
  ASSERT(bound < INT_MAX, "button has no effect");
  return bound;
}

int iterations = 0;
static void dfs(const int *A, const int *goal, int n, int m, int *best, int current, int presses, const int *counters) {
  iterations += 1;

  // Prune on cost
  if (presses >= *best) {
    return;
  }

  // Prune on overshooting goal
  for (int j = 0; j < m; j++) {
    if (counters[j] > goal[j]) {
      return;
    }
  }

  // If all variables assigned, check if goal reached
  if (current == n) {
    for (int j = 0; j < m; j++) {
      if (counters[j] != goal[j]) {
        return;
      }
    }
    *best = presses;
    DBG("best: %d", *best);
    return;
  }

  // Compute upper bound for this button
  int upper_bound = get_upper_bound(A, goal, n, m, current, counters);

  // -----------------------------------------------
  // Feasibility pruning
  // -----------------------------------------------
  //
  // Compute remaining need per counter
  int remaining[m];
  for (int j = 0; j < m; j++) {
    remaining[j] = goal[j] - counters[j];
  }

  // Compute max contributions by remaining buttons
  int max_possible[m];
  for (int j = 0; j < m; j++) {
    max_possible[j] = 0;
  }

  for (int i = current; i < n; i++) {
    int ub_i = get_upper_bound(A, goal, n, m, i, counters);
    for (int j = 0; j < m; j++) {
      if (A[AOC_2D_IDX(i, j, n)]) {
        max_possible[j] += ub_i;
      }
    }
  }

  // Prune if any counter's remaining need cannot be satisfied
  for (int j = 0; j < m; j++) {
    if (max_possible[j] < remaining[j]) {
      return;
    }
  }

  // -----------------------------------------------
  // Lower bound on minimal presses needed
  // -----------------------------------------------

  int remaining_sum = 0;
  for (int j = 0; j < m; j++) {
    remaining_sum += remaining[j];
  }

  int max_effect_per_press = 0;
  for (int i = current; i < n; i++) {
    int effect = 0;
    for (int j = 0; j < m; j++) {
      effect += A[AOC_2D_IDX(i, j, n)];
    }
    if (effect > max_effect_per_press) {
      max_effect_per_press = effect;
    }
  }

  if (max_effect_per_press > 0) {
    int min_possible_presses = (remaining_sum + max_effect_per_press - 1) / max_effect_per_press;

    if (presses + min_possible_presses >= *best) {
      return;
    }
  }

  // Allocate once per DFS level
  int *new_counters = malloc(sizeof(*new_counters) * m);

  // Try all feasible press counts for this button
  for (int count = 0; count <= upper_bound; count++) {

    // Early prune: if presses already too high
    int new_presses = presses + count;
    if (new_presses >= *best)
      break;

    // Build updated counters
    memcpy(new_counters, counters, sizeof(*new_counters) * m);
    for (int j = 0; j < m; j++) {
      new_counters[j] += count * A[AOC_2D_IDX(current, j, n)];
    }

    dfs(A, goal, n, m, best, current + 1, new_presses, new_counters);
  }

  free(new_counters);
}

static void solution2(const char *input, char *const output) {
  int nmachines;
  struct machine *machines = aoc_parse_sequence(&input, &nmachines, sizeof(*machines), 16, parse_machine);

  int solution = 0;
  for (int i = 0; i < nmachines; i++) {
    machine_sort(&machines[i]);

    int n, m;
    int *A = get_coefficient_matrix(&machines[i], &n, &m);
    int *goal = get_jolts_vector(&machines[i]);

    int *counters = malloc(sizeof(*counters) * m);
    memset(counters, 0, sizeof(*counters) * m);

    int best = INT_MAX;
    iterations = 0;
    dfs(A, goal, n, m, &best, 0, 0, counters);
    fprintf(stderr, "case %d/%d done: %d (%d iterations)\n", i + 1, nmachines, best, iterations);
    solution += best;

    free(counters);
    free(A);
    free(goal);
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
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", solution);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
