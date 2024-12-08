#include <aoclib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static int *parse_input(const char *input, int *const width, int *const height) {
  return aoc_parse_grid_digits(&input, height, width);
}

struct point {
  int x, y;
};

static int point_asidx(const struct point p, const int width) { return p.y * width + p.x; }

/* static struct point point_fromidx(const int i, const int width) { */
/*         struct point p; */
/*         p.x = i % width; */
/*         p.y = i / width; */
/*         return p; */
/* } */

static bool point_equal(const struct point a, const struct point b) { return a.x == b.x && a.y == b.y; }

struct prioQueueNode {
  int priority;
  struct point point;
};

struct prioQueue {
  struct prioQueueNode *nodes;
  int size;
  int len;
};

static void prioQueue_init(struct prioQueue *const q) {
  q->len = 0;
  q->size = 64;
  q->nodes = malloc(q->size * sizeof(*q->nodes));
}

static void prioQueue_free(struct prioQueue *const q) {
  free(q->nodes);
  q->nodes = NULL;
}

static bool prioQueue_empty(const struct prioQueue *const q) { return q->len == 0; }

static void prioQueue_push(struct prioQueue *const q, const struct point p, const int prio) {
  if (q->len >= q->size) {
    q->size *= 2;
    q->nodes = realloc(q->nodes, q->size * sizeof(*q->nodes));
  }

  q->nodes[q->len].point = p;
  q->nodes[q->len].priority = prio;

  int i = q->len;
  int p_i = (i - 1) / 2;

  for (;;) {
    if (i == 0) {
      break;
    }
    if (q->nodes[p_i].priority < q->nodes[i].priority) {
      break;
    }

    struct prioQueueNode tmp = q->nodes[i];
    q->nodes[i] = q->nodes[p_i];
    q->nodes[p_i] = tmp;

    i = p_i;
    p_i = (i - 1) / 2;
  }

  q->len += 1;
}

static int prioQueue_pop(struct prioQueue *const q, struct point *const p) {
  ASSERT(q->len > 0, "pop empty queue");

  q->len -= 1;
  int res = q->nodes[0].priority;
  *p = q->nodes[0].point;
  q->nodes[0] = q->nodes[q->len];

  int i = 0;
  int c1_i = 2 * i + 1;
  int c2_i = 2 * i + 2;

  for (;;) {
    if (c1_i >= q->len) {
      break;
    }
    int c_i;
    if (c2_i >= q->len) {
      c_i = c1_i;
    } else {
      if (q->nodes[c1_i].priority < q->nodes[c2_i].priority) {
        c_i = c1_i;
      } else {
        c_i = c2_i;
      }
    }
    if (q->nodes[i].priority < q->nodes[c_i].priority) {
      break;
    }

    struct prioQueueNode tmp = q->nodes[i];
    q->nodes[i] = q->nodes[c_i];
    q->nodes[c_i] = tmp;

    i = c_i;
    c1_i = 2 * i + 1;
    c2_i = 2 * i + 2;
  }

  return res;
}

static int dijkstra(const int *const danger, const int width, const int height, const struct point start,
                    const struct point destination) {
  bool *visited = malloc(width * height * sizeof(*visited));
  int *total_risks = malloc(width * height * sizeof(*total_risks));

  for (int i = 0; i < width * height; i++) {
    visited[i] = false;
    total_risks[i] = INT_MAX;
  }
  total_risks[point_asidx(start, width)] = 0;

  struct prioQueue q;
  prioQueue_init(&q);
  prioQueue_push(&q, start, 0);

  while (!prioQueue_empty(&q)) {
    struct point current;
    int current_risk = prioQueue_pop(&q, &current);
    int currentIdx = point_asidx(current, width);
    if (current_risk > total_risks[currentIdx]) {
      continue;
    }
    if (visited[currentIdx]) {
      continue;
    }
    if (point_equal(current, destination)) {
      break;
    }
    ASSERT(current_risk == total_risks[currentIdx], "somethign is wrong");

    for (int direction = 0; direction < 4; direction++) {
      struct point p;
      switch (direction) {
      case 0: // north
        if (current.y == 0) {
          continue;
        }
        p.x = current.x;
        p.y = current.y - 1;
        break;
      case 1: // south
        if (current.y == height - 1) {
          continue;
        }
        p.x = current.x;
        p.y = current.y + 1;
        break;
      case 2: // west
        if (current.x == 0) {
          continue;
        }
        p.x = current.x - 1;
        p.y = current.y;
        break;
      case 3: // east
        if (current.x == width - 1) {
          continue;
        }
        p.x = current.x + 1;
        p.y = current.y;
        break;
      }

      if (visited[point_asidx(p, width)]) {
        continue;
      }

      int total_risk = current_risk + danger[point_asidx(p, width)];
      if (total_risk < total_risks[point_asidx(p, width)]) {
        total_risks[point_asidx(p, width)] = total_risk;
        prioQueue_push(&q, p, total_risk);
      }
    }

    visited[point_asidx(current, width)] = true;
  }

  int result = total_risks[point_asidx(destination, width)];
  free(visited);
  free(total_risks);
  prioQueue_free(&q);
  return result;
}

static void solution1(const char *const input, char *const output) {
  int width, height;
  int *danger = parse_input(input, &width, &height);

  int lowest_risk =
      dijkstra(danger, width, height, (struct point){.x = 0, .y = 0}, (struct point){.x = width - 1, .y = height - 1});

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", lowest_risk);
  free(danger);
}

static int *enlarge(int *const map, int *const width, int *const height, int factor) {
  int w = *width;
  int h = *height;

  *width = w * factor;
  *height = h * factor;

  int *newmap = malloc(*width * *height * sizeof(*newmap));

  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      for (int x = 0; x < factor; x++) {
        for (int y = 0; y < factor; y++) {
          int value = map[j * w + i] + x + y;
          while (value >= 10) {
            value -= 9;
          }

          int newi = i + w * x;
          int newj = j + h * y;
          int idx = newj * *width + newi;

          newmap[idx] = value;
        }
      }
    }
  }

  free(map);
  return newmap;
}

static void solution2(const char *const input, char *const output) {
  int width, height;
  int *danger = parse_input(input, &width, &height);

  danger = enlarge(danger, &width, &height, 5);

  int lowest_risk =
      dijkstra(danger, width, height, (struct point){.x = 0, .y = 0}, (struct point){.x = width - 1, .y = height - 1});

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", lowest_risk);
  free(danger);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
