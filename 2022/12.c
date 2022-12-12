#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

struct point {
        int x;
        int y;
};
#define IDX(p, w) ((p).y*(w)+(p).x)

#define QUEUE_SIZE 1024
struct queue {
        struct point elements[QUEUE_SIZE];
        int head, tail;
};

static void queue_init(struct queue *q) {
        q->head = q->tail = 0;
}

static void queue_push(struct queue *q, struct point s) {
        ASSERT(q->head + 1 != q->tail, "queue full");
        q->elements[q->head] = s;
        q->head = (q->head + 1) % QUEUE_SIZE;
}

static bool queue_empty(const struct queue *q) {
        return q->head == q->tail;
}

static struct point queue_pop(struct queue *q) {
        ASSERT(q->head != q->tail, "queue empty");
        struct point r = q->elements[q->tail];
        q->tail = (q->tail + 1) % QUEUE_SIZE;
        return r;
}

static void populate_cardinals(struct point cardinals[4], struct point p, int width, int height, const int *map, int *memory) {
        for (int i=0; i<4; i++) {
                cardinals[i] = p;
        }
        cardinals[0].x++;
        cardinals[1].x--;
        cardinals[2].y++;
        cardinals[3].y--;

        const struct point invalid = {.x=-1, .y=-1};
        for (int i=0; i<4; i++) {
                struct point cardinal = cardinals[i];
                if (cardinal.x < 0 || cardinal.x >= width ||
                    cardinal.y < 0 || cardinal.y >= height) {
                        cardinals[i] = invalid;
                        continue;
                }

                int next = map[IDX(cardinal, width)];
                int prev = map[IDX(p, width)];
                if (next - prev < -1) {
                        cardinals[i] = invalid;
                        continue;
                }

                if (memory[IDX(cardinal, width)] >= 0) {
                        cardinals[i] = invalid;
                        continue;
                }
        }
}

static int bfs(const int *map, int width, int height, struct point start, struct queue *q, int *memory) {
        int res = memory[IDX(start, width)];
        if (res >= 0) {
                return res;
        }
        
        while (!queue_empty(q)) {
                struct point here = queue_pop(q);
                int steps_from_here = memory[IDX(here, width)];
                ASSERT(steps_from_here >= 0, "logic error");
                
                struct point cardinals[4];
                populate_cardinals(cardinals, here, width, height, map, memory);

                for (int i=0; i<4; i++) {
                        struct point cardinal = cardinals[i];
                        if (cardinal.x < 0) {
                                continue;
                        }

                        //DBG("%d,%d (%d) -> %d,%d (%d)", here.x, here.y, map[IDX(here, width)], cardinal.x, cardinal.y, map[IDX(cardinal, width)]);
                        memory[IDX(cardinal, width)] = steps_from_here + 1;
                        if (cardinal.x == start.x && cardinal.y == start.y) {
                                return memory[IDX(cardinal, width)];
                        }

                        queue_push(q, cardinal);
                }
        }

        // unreachable
        return INT_MAX;
}

static int *parse_input(const char *const input, int *width, int *height,
                        struct point *start, struct point *end) {
        int len = 0;
        int cap = 64;
        int *map = malloc(cap * sizeof(*map));
        
        *width = 0;
        for (int i=0;; i++) {
                char c = input[i];
                if (c == '\0') {
                        break;
                }
                if (c == '\n') {
                        if (*width == 0) {
                                *width = i;
                        }
                        continue;
                }

                if (len >= cap) {
                        cap *= 2;
                        map = realloc(map, cap * sizeof(*map));
                }
                if (c == 'S' || c == 'E') {
                        struct point p;
                        if (*width == 0) {
                                p.y = 0;
                                p.x = len;
                        } else {
                                p.y = len / *width;
                                p.x = len % *width;
                        }
                        if (c == 'S') {
                                c = 'a';
                                *start = p;
                        } else if (c == 'E') {
                                c = 'z';
                                *end = p;
                        }
                } else if (c < 'a' || c > 'z') {
                        FAIL("parse error");
                }
                map[len++] = c - 'a';
        }

        ASSERT(len % *width == 0, "parse error");
        *height = len / *width;
        
#ifdef DEBUG
        for (int j=0; j<*height; j++) {
                for (int i=0; i<*width; i++) {
                        char c;
                        if (i == start->x && j == start->y) {
                                c = 'S';
                        } else if (i == end->x && j == end->y) {
                                c = 'E';
                        } else {
                                c = 'a' + map[j**width+i];
                        }
                        fputc(c, stderr);
                }
                fputc('\n', stderr);
        }
#endif

        return map;
}

static void solution1(const char *const input, char *const output) {
        int width;
        int height;
        struct point start;
        struct point end;
        int *map = parse_input(input, &width, &height, &start, &end);

        int *memory = malloc(width * height * sizeof(*memory));
        for (int i=0; i<width * height; i++) {
                memory[i] = -1;
        }
        
        struct queue q;
        queue_init(&q);
        queue_push(&q, end);
        memory[IDX(end, width)] = 0;
        
        int steps = bfs(map, width, height, start, &q, memory);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", steps);
        free(map);
        free(memory);
}

static void solution2(const char *const input, char *const output) {
        int width;
        int height;
        struct point start;
        struct point end;
        int *map = parse_input(input, &width, &height, &start, &end);

        int *memory = malloc(width * height * sizeof(*memory));
        for (int i=0; i<width * height; i++) {
                memory[i] = -1;
        }
        
        struct queue q;
        queue_init(&q);
        queue_push(&q, end);
        memory[IDX(end, width)] = 0;
        
        int minsteps = INT_MAX;
        for (int j=0; j<height; j++) {
                start.y = j;
                for (int i=0; i<width; i++) {
                        start.x = i;
                        if (map[IDX(start, width)] == 0) {
                                int steps = bfs(map, width, height, start, &q, memory);
                                if (steps < minsteps) {
                                        minsteps = steps;
                                }
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", minsteps);
        free(map);
        free(memory);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
