#include <aoclib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define IDX(array, x, y, width) ((array)[(x) + (y) * (width)])
#define QUEUE_MAX_SIZE (1 << 10)
// #define TRACE

static char *parse_input(const char *input, int *width, int *height) {
  int len = 0;
  int cap = 16;
  char *list = malloc(sizeof(*list) * cap);

  int w = 0;
  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    char c = *input;
    if (c == '\n') {
      if (w == 0) {
        w = len;
      }
    } else {
      list[len++] = c;
    }
    input++;
  }

  *width = w;
  *height = len / w;
  return list;
}

struct state {
  int x;
  int y;
  int steps;
  bool *visited;
};

static void state_init(struct state *state, int x, int y, int width, int height) {
  state->x = x;
  state->y = y;
  state->steps = 0;

  size_t s = sizeof(*state->visited) * width * height;
  state->visited = malloc(s);
  memset(state->visited, 0, s);
}

static void state_duplicate(struct state *dst, const struct state *src, int width, int height) {
  dst->x = src->x;
  dst->y = src->y;
  dst->steps = src->steps;

  size_t s = sizeof(*dst->visited) * width * height;
  dst->visited = malloc(s);
  memcpy(dst->visited, src->visited, s);
}

static void state_free(struct state *state) { free(state->visited); }

struct stack {
  int len;
  int cap;
  struct state *list;
};

static void stack_init(struct stack *stack) {
  stack->len = 0;
  stack->cap = 16;
  stack->list = malloc(sizeof(*stack->list) * stack->cap);
}

static void stack_push(struct stack *stack, struct state element) {
  if (stack->len >= stack->cap) {
    stack->cap *= 2;
    stack->list = realloc(stack->list, sizeof(*stack->list) * stack->cap);
  }

  stack->list[stack->len++] = element;
}

static struct state stack_pop(struct stack *stack) { return stack->list[--stack->len]; }

static bool stack_empty(const struct stack *stack) { return stack->len == 0; }

static void stack_free(struct stack *stack) { free(stack->list); }

static int worst_path(const char *map, int width, int height) {
  const int goal_x = width - 2;
  const int goal_y = height - 1;

  struct state state;
  state_init(&state, 1, 0, width, height);

  struct stack stack;
  stack_init(&stack);
  stack_push(&stack, state);

  int worst = 0;
  while (!stack_empty(&stack)) {
    state = stack_pop(&stack);
    if (IDX(state.visited, state.x, state.y, width)) {
      state_free(&state);
      continue;
    }
    IDX(state.visited, state.x, state.y, width) = true;

    char c = IDX(map, state.x, state.y, width);
    state.steps++;
    if (c == '^') {
      state.y--;
      stack_push(&stack, state);
    } else if (c == '<') {
      state.x--;
      stack_push(&stack, state);
    } else if (c == 'v') {
      state.y++;
      stack_push(&stack, state);
    } else if (c == '>') {
      state.x++;
      stack_push(&stack, state);
    } else {
      ASSERT(c == '.', "logic error");
      for (int i = 0; i < 4; i++) {
        struct state new = state;

        switch (i) {
        case 0:
          new.y--;
          break;
        case 1:
          new.x--;
          break;
        case 2:
          new.y++;
          break;
        case 3:
          new.x++;
          break;
        }

        if (new.x == goal_x &&new.y == goal_y) {
          if (new.steps > worst) {
            worst = new.steps;
          }
          continue;
        }
        if (new.x < 0 || new.x >= width || new.y < 0 || new.y >= height) {
          continue;
        }
        if (IDX(map, new.x, new.y, width) == '#') {
          continue;
        }

        if (IDX(new.visited, new.x, new.y, width)) {
          continue;
        }

        struct state new2;
        state_duplicate(&new2, &state, width, height);
        new2.x = new.x;
        new2.y = new.y;
        stack_push(&stack, new2);
        DBG("%d", stack.len);
      }
      state_free(&state);
    }
  }

  stack_free(&stack);
  return worst;
}

static void solution1(const char *const input, char *const output) {
  int width, height;
  char *map = parse_input(input, &width, &height);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", worst_path(map, width, height));
  free(map);
}

struct point {
  int x;
  int y;
};

struct queue_elemnt {
  struct point p;
  int distance;
};

struct queue {
  int front;
  int tail;
  struct queue_elemnt elements[QUEUE_MAX_SIZE];
};

static void queue_init(struct queue *q) {
  q->front = 0;
  q->tail = 0;
}

static void queue_push(struct queue *q, struct queue_elemnt s) {
  q->elements[q->tail] = s;
  q->tail++;
  q->tail %= QUEUE_MAX_SIZE;
  ASSERT(q->tail != q->front, "queue full");
}

static struct queue_elemnt queue_pop(struct queue *q) {
  ASSERT(q->tail != q->front, "queue empty");
  struct queue_elemnt s = q->elements[q->front];
  q->front++;
  q->front %= QUEUE_MAX_SIZE;
  return s;
}

static bool queue_empty(const struct queue *q) { return q->front == q->tail; }

static int direct_distance(const char *map, int width, int height, struct point p1, struct point p2) {
  if (p1.x == p2.x && p1.y == p2.y) {
    return 0;
  }

  bool *visited = malloc(sizeof(*visited) * width * height);
  memset(visited, 0, sizeof(*visited) * width * height);

  struct queue q;
  queue_init(&q);

  for (int i = 0; i < 4; i++) {
    struct point p = p1;
    switch (i) {
    case 0:
      p.y--;
      break;
    case 1:
      p.x--;
      break;
    case 2:
      p.y++;
      break;
    case 3:
      p.x++;
      break;
    }
    if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) {
      continue;
    }
    if (IDX(map, p.x, p.y, width) == '.') {
      struct queue_elemnt e;
      e.p = p;
      e.distance = 1;
      queue_push(&q, e);
    }
  }
  IDX(visited, p1.x, p1.y, width) = true;

  while (!queue_empty(&q)) {
    struct queue_elemnt e = queue_pop(&q);
    struct point p = e.p;
    if (IDX(visited, p.x, p.y, width)) {
      continue;
    }
    IDX(visited, p.x, p.y, width) = true;

    if (p.x == p2.x && p.y == p2.y) {
      free(visited);
      return e.distance;
    }

    bool foundmany = false;
    bool found = false;
    struct point next;
    for (int i = 0; i < 4; i++) {
      struct point can = p;
      switch (i) {
      case 0:
        can.y--;
        break;
      case 1:
        can.x--;
        break;
      case 2:
        can.y++;
        break;
      case 3:
        can.x++;
        break;
      }
      if (can.x < 0 || can.x >= width || can.y < 0 || can.y >= height) {
        continue;
      }
      if (IDX(map, can.x, can.y, width) == '.' && !IDX(visited, can.x, can.y, width)) {
        if (found) {
          foundmany = true;
          break;
        } else {
          found = true;
          next = can;
        }
      }
    }
    if (found && !foundmany) {
      struct queue_elemnt nexte;
      nexte.p = next;
      nexte.distance = e.distance + 1;
      queue_push(&q, nexte);
    }
  }

  free(visited);
  return -1;
}

struct path {
  bool *contains;
  int last_added;
  int length;

#ifdef DEBUG
  int *history;
  int historylen;
#endif
};

static void path_init(struct path *p, int size, int start) {
  p->contains = malloc(sizeof(*p->contains) * size);
  for (int i = 0; i < size; i++) {
    p->contains[i] = false;
  }
  p->last_added = start;
  p->length = 0;

#ifdef DEBUG
  p->history = malloc(sizeof(*p->contains) * size);
  p->history[0] = start;
  p->historylen = 1;
#endif
}

static void path_duplicate(struct path *dst, const struct path *src, int size) {
  dst->contains = malloc(sizeof(*dst->contains) * size);
  for (int i = 0; i < size; i++) {
    dst->contains[i] = src->contains[i];
  }
  dst->last_added = src->last_added;
  dst->length = src->length;

#ifdef DEBUG
  dst->history = malloc(sizeof(*dst->history) * size);
  for (int i = 0; i < src->historylen; i++) {
    dst->history[i] = src->history[i];
  }
  dst->historylen = src->historylen;
#endif
}

static void path_free(struct path *p) {
  free(p->contains);
#ifdef DEBUG
  free(p->history);
#endif
}

static void path_add(struct path *p, int node, int weight) {
  p->contains[node] = true;
  p->last_added = node;
  p->length += weight;
#ifdef DEBUG
  p->history[p->historylen++] = node;
#endif
}

static bool path_contains(const struct path *p, int node) { return p->contains[node]; }

static int path_latest(const struct path *p) { return p->last_added; }

static void path_print(const struct path *p, int end) {
#ifdef TRACE
  for (int i = 0; i < p->historylen; i++) {
    fprintf(stderr, "%d", p->history[i]);
    if (i < p->historylen - 1) {
      fputs(" -> ", stderr);
    }
  }
  if (p->history[p->historylen - 1] == end) {
    fprintf(stderr, " OK! LENGTH=%d", p->length);
  }
  fputc('\n', stderr);
#elif defined(DEBUG)
  if (p->history[p->historylen - 1] == end) {
    for (int i = 0; i < p->historylen; i++) {
      fprintf(stderr, "%d", p->history[i]);
      if (i < p->historylen - 1) {
        fputs(" -> ", stderr);
      }
    }
    fprintf(stderr, " LENGTH=%d\n", p->length);
  }
#else
  (void)p;
  (void)end;
#endif
}

static int find_longest_path(int *distances, int nnodes, int start, int end) {
  int stackcap = 16;
  int stacklen = 0;
  struct path *stack = malloc(sizeof(*stack) * stackcap);

  path_init(stack, nnodes, start);
  stacklen++;

  int best = 0;
  while (stacklen > 0) {
    struct path curr = stack[--stacklen];
    path_print(&curr, end);

    if (path_contains(&curr, end)) {
      if (curr.length > best) {
        best = curr.length;
        path_free(&curr);
        continue;
      }
    }

    for (int i = 0; i < nnodes; i++) {
      int d = IDX(distances, path_latest(&curr), i, nnodes);
      if (d > 0 && !path_contains(&curr, i)) {
        struct path next;
        path_duplicate(&next, &curr, nnodes);
        path_add(&next, i, d);

        if (stacklen >= stackcap) {
          stackcap *= 2;
          stack = realloc(stack, sizeof(*stack) * stackcap);
        }
        stack[stacklen++] = next;
      }
    }
    path_free(&curr);
  }

  free(stack);
  return best;
}

static void solution2(const char *const input, char *const output) {
  int width, height;
  char *map = parse_input(input, &width, &height);

  // remove slopes
  for (int i = 0; i < width * height; i++) {
    char c = map[i];
    if (c == '^' || c == '<' || c == 'v' || c == '>') {
      map[i] = '.';
    }
  }

  // each crossing of paths is a node in the graph (plus start and end)
  int nnodes = 2;
  int cap = 8;
  struct point *nodes = malloc(sizeof(*nodes) * cap);
  nodes[0].x = 1;
  nodes[0].y = 0;
  nodes[1].x = width - 2;
  nodes[1].y = height - 1;
  for (int j = 1; j < height - 1; j++) {
    for (int i = 1; i < width - 1; i++) {
      char c = IDX(map, i, j, width);
      if (c != '.') {
        continue;
      }

      char north = IDX(map, i, j - 1, width);
      char west = IDX(map, i - 1, j, width);
      char south = IDX(map, i, j + 1, width);
      char east = IDX(map, i + 1, j, width);
      int count = (north == '.') + (west == '.') + (south == '.') + (east == '.');

      if (count == 1) {
        FAIL("dead end?");
      } else if (count > 2) {
        if (nnodes >= cap) {
          cap *= 2;
          nodes = realloc(nodes, sizeof(*nodes) * cap);
        }
        struct point p = {.x = i, .y = j};
        nodes[nnodes++] = p;
      }
    }
  }
  for (int i = 0; i < nnodes; i++) {
    DBG("%d,%d", nodes[i].x, nodes[i].y);
  }

  // distances between each node (if directly connected)
  int *node_distances = malloc(sizeof(*node_distances) * nnodes * nnodes);
  for (int i = 0; i < nnodes; i++) {
    for (int j = 0; j < nnodes; j++) {
      IDX(node_distances, i, j, nnodes) = direct_distance(map, width, height, nodes[i], nodes[j]);
    }
  }
  for (int i = 0; i < nnodes; i++) {
    for (int j = i; j < nnodes; j++) {
      int d = IDX(node_distances, i, j, nnodes);
      if (d > 0) {
        DBG("%d,%d -> %d,%d = %d", nodes[i].x, nodes[i].y, nodes[j].x, nodes[j].y, d);
      }
    }
  }

  /* Adjacency matrix, for funsies
  for (int i=0; i<nnodes; i++) {
          for (int j=0; j<nnodes; j++) {
                  int d = IDX(node_distances, i, j, nnodes);
                  if (d < 0) {
                          d = 0;
                  }
                  fprintf(stderr, "%d", d);
                  if (j < nnodes-1) {
                          fputc(' ', stderr);
                  }
          }
          fputc('\n', stderr);
  }
  */

  // we have a graph, find the longest path
  int res = find_longest_path(node_distances, nnodes, 0, 1);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  free(map);
  free(nodes);
  free(node_distances);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
