#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define DIM_VERT ('z' - 'a' + 1)
#define MAX_VERT (DIM_VERT * DIM_VERT)
#define FIRST_LETTER(n) ((n) / DIM_VERT + 'a')
#define SECOND_LETTER(n) ((n) % DIM_VERT + 'a')

struct adjacency_data {
  int vertex;
  const int *adjacent_vertices;
  int len_adjacent_vertices;
};

static struct adjacency_data *parse_input(const char *input, int *len_vertices,
                                          bool sparse_adjacency_matrix[MAX_VERT][MAX_VERT]) {
  while (*input != 0) {
    int n = (input[0] - 'a') * DIM_VERT + input[1] - 'a';
    input += 2;
    aoc_expect_char(&input, '-');
    int m = (input[0] - 'a') * DIM_VERT + input[1] - 'a';
    input += 2;
    aoc_skip_space(&input);

    sparse_adjacency_matrix[n][m] = true;
    sparse_adjacency_matrix[m][n] = true;
  }

  struct aoc_dynarr adjacency_list;
  aoc_dynarr_init(&adjacency_list, sizeof(struct adjacency_data), 16);
  for (int i = 0; i < MAX_VERT; i++) {
    struct aoc_dynarr adjacent_nodes;
    aoc_dynarr_init(&adjacent_nodes, sizeof(int), 4);
    for (int j = 0; j < MAX_VERT; j++) {
      if (sparse_adjacency_matrix[i][j]) {
        int *n = aoc_dynarr_grow(&adjacent_nodes, 1);
        *n = j;
      }
    }

    if (adjacent_nodes.len == 0) {
      // no adjacent nodes means the node doesn't exist, since the input is a list of adjacencies
      aoc_dynarr_free(&adjacent_nodes);
      continue;
    }

    struct adjacency_data *adj = aoc_dynarr_grow(&adjacency_list, 1);
    adj->vertex = i;
    adj->adjacent_vertices = adjacent_nodes.data;
    adj->len_adjacent_vertices = adjacent_nodes.len;
  }

  *len_vertices = adjacency_list.len;
  return adjacency_list.data;
}

static void solution1(const char *input, char *const output) {
  static bool sparse_adjacency_matrix[MAX_VERT][MAX_VERT];
  int len_vertices;
  const struct adjacency_data *adjacency_list = parse_input(input, &len_vertices, sparse_adjacency_matrix);

  int count = 0;
  for (int i = 0; i < len_vertices; i++) {
    struct adjacency_data adj = adjacency_list[i];
    if (adj.len_adjacent_vertices < 2) {
      continue;
    }

    int u = adj.vertex;
    for (int j = 0; j < adj.len_adjacent_vertices; j++) {
      int v = adj.adjacent_vertices[j];
      if (u > v) {
        continue;
      }
      for (int k = j + 1; k < adj.len_adjacent_vertices; k++) {
        int w = adj.adjacent_vertices[k];
        if (FIRST_LETTER(u) != 't' && FIRST_LETTER(v) != 't' && FIRST_LETTER(w) != 't') {
          continue;
        }
        if (u > w || v > w) {
          continue;
        }
        if (!sparse_adjacency_matrix[v][w]) {
          continue;
        }
        count++;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  for (int i = 0; i < len_vertices; i++) {
    free((void *)adjacency_list[i].adjacent_vertices);
  }
  free((void *)adjacency_list);
#pragma GCC diagnostic pop
}

static bool is_graph_complete(int *graph, int graph_len, int ignore, bool adjacency[MAX_VERT][MAX_VERT]) {
  for (int i = 0; i < graph_len; i++) {
    if (i == ignore) {
      continue;
    }
    for (int j = 0; j < graph_len; j++) {
      if (j == ignore) {
        continue;
      }
      if (i == j) {
        continue;
      }
      if (!adjacency[graph[i]][graph[j]]) {
        return false;
      }
    }
  }
  return true;
}

static void solution2(const char *input, char *const output) {
  static bool sparse_adjacency_matrix[MAX_VERT][MAX_VERT];
  int len_vertices;
  struct adjacency_data *adjacency_list = parse_input(input, &len_vertices, sparse_adjacency_matrix);

  // Turns out, this is a regular graph.
#ifdef DEBUG
  for (int i = 0; i < len_vertices; i++) {
    struct adjacency_data *adj = &adjacency_list[i];
    fprintf(stderr, "%d: %d\n", adj->vertex, adj->len_adjacent_vertices);
  }
#endif
  // And so is the example graph. And that one had a d-clique with d being the
  // degree of the graph. So this one probably does too.
  //
  // Let's try by finding a vertex such that its neighbors plus itself form a
  // fully connected graph IF we remove one vertex.

  static int max_clique[MAX_VERT];
  int max_clique_len = adjacency_list[0].len_adjacent_vertices + 1;
  int max_clique_ignore = 0;
  for (int i = 0; i < len_vertices; i++) {
    struct adjacency_data adj = adjacency_list[i];

    memcpy(max_clique, adj.adjacent_vertices, sizeof(*adj.adjacent_vertices) * adj.len_adjacent_vertices);
    max_clique[adj.len_adjacent_vertices] = adj.vertex;

    bool found = false;
    for (int ignore = 0; ignore < max_clique_len; ignore++) {
      if (is_graph_complete(max_clique, max_clique_len, ignore, sparse_adjacency_matrix)) {
        found = true;
        max_clique_ignore = max_clique[ignore];
        break;
      }
    }
    if (found) {
      break;
    }
  }

  qsort(max_clique, max_clique_len, sizeof(int), aoc_cmp_int);

  struct aoc_dynarr text;
  aoc_dynarr_init(&text, sizeof(char), 16);
  for (int i = 0; i < max_clique_len; i++) {
    if (max_clique[i] == max_clique_ignore) {
      continue;
    }
    char *name = aoc_dynarr_grow(&text, 3);
    name[0] = FIRST_LETTER(max_clique[i]);
    name[1] = SECOND_LETTER(max_clique[i]);
    name[2] = ',';
  }
  ((char *)text.data)[text.len - 1] = '\0';

  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", (char *)text.data);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
  for (int i = 0; i < len_vertices; i++) {
    free((void *)adjacency_list[i].adjacent_vertices);
  }
  free((void *)adjacency_list);
#pragma GCC diagnostic pop
  aoc_dynarr_free(&text);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
