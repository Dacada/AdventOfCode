#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

static unsigned *parse_input(const char *input, size_t *const width, size_t *const height) {
        size_t size = 64;
        unsigned *matrix = malloc(size * sizeof(*matrix));
        size_t i = 0;

        *width = 0;
        while (*input != '\n') {
                if (i >= size) {
                        size *= 2;
                        matrix = realloc(matrix, size * sizeof(*matrix));
                }
                ASSERT(isdigit(*input), "parse error");
                matrix[i++] = *input - '0';
                *width += 1;
                input += 1;
        }

        while (*input != '\0') {
                if (i >= size) {
                        size *= 2;
                        matrix = realloc(matrix, size * sizeof(*matrix));
                }
                if (*input == '\n') {
                        input += 1;
                        if (*input == '\0') {
                                break;
                        }
                }
                ASSERT(isdigit(*input), "parse error");
                matrix[i++] = *input - '0';
                input += 1;
        }

        *height = i / *width;
        return matrix;
}

struct point {
        size_t x, y;
};

static size_t point_asidx(const struct point p, const size_t width) {
        return p.y * width + p.x;
}

/* static struct point point_fromidx(const size_t i, const size_t width) { */
/*         struct point p; */
/*         p.x = i % width; */
/*         p.y = i / width; */
/*         return p; */
/* } */

static bool point_equal(const struct point a, const struct point b) {
        return a.x == b.x && a.y == b.y;
}


struct prioQueueNode {
        unsigned priority;
        struct point point;
};

struct prioQueue {
        struct prioQueueNode *nodes;
        size_t size;
        size_t len;
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

static bool prioQueue_empty(const struct prioQueue *const q) {
        return q->len == 0;
}

static void prioQueue_push(struct prioQueue *const q, const struct point p, const unsigned prio) {
        if (q->len >= q->size) {
                q->size *= 2;
                q->nodes = realloc(q->nodes, q->size * sizeof(*q->nodes));
        }
        
        q->nodes[q->len].point = p;
        q->nodes[q->len].priority = prio;

        size_t i = q->len;
        size_t p_i = (i-1)/2;

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
                p_i = (i-1)/2;
        }
        
        q->len += 1;
}

static unsigned prioQueue_pop(struct prioQueue *const q, struct point *const p) {
        ASSERT(q->len > 0, "pop empty queue");
        
        q->len -= 1;
        unsigned res = q->nodes[0].priority;
        *p = q->nodes[0].point;
        q->nodes[0] = q->nodes[q->len];
        
        size_t i = 0;
        size_t c1_i = 2*i+1;
        size_t c2_i = 2*i+2;

        for (;;) {
                if (c1_i >= q->len) {
                        break;
                }
                size_t c_i;
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
                c1_i = 2*i+1;
                c2_i = 2*i+2;
        }

        return res;
}

static unsigned dijkstra(const unsigned *const danger, const size_t width, const size_t height, const struct point start, const struct point destination) {
        bool *visited = malloc(width * height * sizeof(*visited));
        unsigned *total_risks = malloc(width * height * sizeof(*total_risks));
        
        for (size_t i=0; i<width*height; i++) {
                visited[i] = false;
                total_risks[i] = UINT_MAX;
        }
        total_risks[point_asidx(start, width)] = 0;

        struct prioQueue q;
        prioQueue_init(&q);
        prioQueue_push(&q, start, 0);

        while (!prioQueue_empty(&q)) {
                struct point current;
                unsigned current_risk = prioQueue_pop(&q, &current);
                size_t currentIdx = point_asidx(current, width);
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
                
                for (int direction=0; direction<4; direction++) {
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
                                if (current.y == height-1) {
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
                                if (current.x == width-1) {
                                        continue;
                                }
                                p.x = current.x + 1;
                                p.y = current.y;
                                break;
                        }

                        if (visited[point_asidx(p, width)]) {
                                continue;
                        }

                        unsigned total_risk = current_risk + danger[point_asidx(p, width)];
                        if (total_risk < total_risks[point_asidx(p, width)]) {
                                total_risks[point_asidx(p, width)] = total_risk;
                                prioQueue_push(&q, p, total_risk);
                        }
                }

                visited[point_asidx(current, width)] = true;
        }

        unsigned result = total_risks[point_asidx(destination, width)];
        free(visited);
        free(total_risks);
        prioQueue_free(&q);
        return result;
}

static void solution1(const char *const input, char *const output) {
        size_t width, height;
        unsigned *danger = parse_input(input, &width, &height);

        unsigned lowest_risk = dijkstra(danger, width, height,
                                       (struct point){.x=0, .y=0}, (struct point){.x=width-1, .y=height-1});
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", lowest_risk);
        free(danger);
}

static unsigned *enlarge(unsigned *const map, size_t *const width, size_t *const height, unsigned factor) {
        size_t w = *width;
        size_t h = *height;

        *width = w*factor;
        *height = h*factor;
        
        unsigned *newmap = malloc(*width * *height * sizeof(*newmap));

        for (size_t j=0; j<h; j++) {
                for (size_t i=0; i<w; i++) {
                        for (unsigned x=0; x<factor; x++) {
                                for (unsigned y=0; y<factor; y++) {
                                        unsigned value = map[j*w+i] + x + y;
                                        while (value >= 10) {
                                                value -= 9;
                                        }

                                        size_t newi = i + w*x;
                                        size_t newj = j + h*y;
                                        size_t idx = newj * *width + newi;
                                        
                                        newmap[idx] = value;
                                }
                        }
                }
        }

        free(map);
        return newmap;
}

static void solution2(const char *const input, char *const output) {
        size_t width, height;
        unsigned *danger = parse_input(input, &width, &height);

        danger = enlarge(danger, &width, &height, 5);

        unsigned lowest_risk = dijkstra(danger, width, height,
                                       (struct point){.x=0, .y=0}, (struct point){.x=width-1, .y=height-1});
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", lowest_risk);
        free(danger);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
