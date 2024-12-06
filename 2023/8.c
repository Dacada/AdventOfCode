#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct node {
  char name[3];
  char left_name[3];
  char right_name[3];
  struct node *left;
  struct node *right;
};

static void parse_directions(const char **input, const char **directions, int *ndirections) {
  int i;
  for (i = 0; (*input)[i] != '\n'; i++)
    ;
  *ndirections = i;
  *directions = *input;
  *input += i;
  while (isspace(**input)) {
    *input += 1;
  }
}

static void assert_string(const char **str, const char *expected) {
  int n = strlen(expected);
  ASSERT(strncmp(*str, expected, n) == 0, "parse error '%s' '%s'", *str, expected);
  *str += n;
}

static void parse_node(const char **input, struct node *node) {
  for (int i = 0; i < 3; i++) {
    node->name[i] = **input;
    *input += 1;
  }
  assert_string(input, " = (");
  for (int i = 0; i < 3; i++) {
    node->left_name[i] = **input;
    *input += 1;
  }
  assert_string(input, ", ");
  for (int i = 0; i < 3; i++) {
    node->right_name[i] = **input;
    *input += 1;
  }
  assert_string(input, ")");
  if (**input == '\n') {
    *input += 1;
  }
}

static void parse_nodes(const char **input, struct node **nodes, int *nnodes) {
  int len = 0;
  int cap = 16;
  struct node *res = malloc(sizeof(*res) * cap);

  while (**input != '\0') {
    if (len >= cap) {
      cap *= 2;
      res = realloc(res, sizeof(*res) * cap);
    }
    parse_node(input, res + len);
    len++;
  }

  *nodes = res;
  *nnodes = len;
}

static int node_cmp(const void *n1, const void *n2) {
  const struct node *node1 = n1;
  const struct node *node2 = n2;
  return strncmp(node1->name, node2->name, 3);
}

static void sort_nodes(struct node *nodes, int nnodes) { qsort(nodes, nnodes, sizeof(*nodes), node_cmp); }

static void assign_nodes(struct node *nodes, int nnodes) {
  struct node key;

  for (int i = 0; i < nnodes; i++) {
    memcpy(key.name, nodes[i].left_name, 3);
    nodes[i].left = bsearch(&key, nodes, nnodes, sizeof(*nodes), node_cmp);

    memcpy(key.name, nodes[i].right_name, 3);
    nodes[i].right = bsearch(&key, nodes, nnodes, sizeof(*nodes), node_cmp);
  }
}

static void parse_input(const char *input, const char **directions, int *ndirections, struct node **nodes,
                        int *nnodes) {
  parse_directions(&input, directions, ndirections);
  parse_nodes(&input, nodes, nnodes);
  ASSERT(*input == '\0', "parse error");
  sort_nodes(*nodes, *nnodes);
  assign_nodes(*nodes, *nnodes);
}

static void solution1(const char *const input, char *const output) {
  const char *directions;
  int ndirections;
  struct node *nodes;
  int nnodes;
  parse_input(input, &directions, &ndirections, &nodes, &nnodes);

  struct node *first;
  {
    struct node key;
    memcpy(key.name, "AAA", 3);
    first = bsearch(&key, nodes, nnodes, sizeof(*nodes), node_cmp);
  }

  int i = 0;
  int result = 0;
  struct node *n = first;
  while (strncmp(n->name, "ZZZ", 3) != 0) {
    char c = directions[i];
    if (c == 'L') {
      n = n->left;
    } else if (c == 'R') {
      n = n->right;
    } else {
      FAIL("bad direction");
    }
    i = (i + 1) % ndirections;
    result++;
  }

  free(nodes);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void find_starts(struct node ***starts, int *nstarts, struct node *nodes, int nnodes) {
  int len = 0;
  int cap = 16;
  struct node **res = malloc(sizeof(*res) * cap);

  for (int i = 0; i < nnodes; i++) {
    if (len >= cap) {
      cap *= 2;
      res = realloc(res, sizeof(*res) * cap);
    }
    if (nodes[i].name[2] == 'A') {
      res[len++] = nodes + i;
    }
  }

  *starts = res;
  *nstarts = len;
}

__attribute__((const)) static long gcd(long a, long b) {
  while (a != b) {
    if (a > b) {
      a -= b;
    } else {
      b -= a;
    }
  }
  return a;
}

__attribute__((const)) static long lcm(long a, long b) { return (a * b) / gcd(a, b); }

static void solution2(const char *const input, char *const output) {
  const char *directions;
  int ndirections;
  struct node *nodes;
  int nnodes;
  parse_input(input, &directions, &ndirections, &nodes, &nnodes);

  struct node **starts;
  int nstarts;
  find_starts(&starts, &nstarts, nodes, nnodes);

  int *first_z = malloc(sizeof(*first_z) * nstarts);
  int *second_z = malloc(sizeof(*second_z) * nstarts);
  for (int i = 0; i < nstarts; i++) {
    first_z[i] = second_z[i] = -1;
  }

  int i = 0;
  int count = 0;
  for (;;) {
    char c = directions[i];
    bool maybe_done = false;
    for (int j = 0; j < nstarts; j++) {
      if (c == 'L') {
        starts[j] = starts[j]->left;
      } else if (c == 'R') {
        starts[j] = starts[j]->right;
      } else {
        FAIL("bad direction");
      }

      if (starts[j]->name[2] == 'Z') {
        if (first_z[j] < 0) {
          first_z[j] = count;
        } else if (second_z[j] < 0) {
          second_z[j] = count;
          maybe_done = true;
        }
      }
    }
    i = (i + 1) % ndirections;
    count++;

    bool done = maybe_done;
    if (maybe_done) {
      for (int j = 0; j < nstarts; j++) {
        if (second_z[j] < 0) {
          done = false;
        }
      }
    }
    if (done) {
      break;
    }
  }

  int *periods = malloc(sizeof(*periods) * nstarts);
  for (int j = 0; j < nstarts; j++) {
    periods[j] = second_z[j] - first_z[j];
    DBG("%d - %d = %d", second_z[j], first_z[j], periods[j]);
  }
  free(first_z);
  free(second_z);

  long result = lcm(periods[0], periods[1]);
  for (int j = 2; j < nstarts; j++) {
    result = lcm(result, periods[j]);
  }
  free(periods);

  free(nodes);
  free(starts);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
