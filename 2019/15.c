#ifdef DEBUG
#define _DEFAULT_SOURCE
#include <unistd.h>
#endif

#include "intcode.h"
#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define MAPSIZE 64
#define QUEUESIZE 256

#define GOALTOKEN 12345

#define INDEX(map,pos) map[pos.y][pos.x]

struct vec2 {
        int x, y;
};

struct vec3 {
        int x, y, z;
};

enum direction {
        NORTH=1,
        SOUTH=2,
        WEST=3,
        EAST=4
};

enum status {
        WALL=0,
        FREE=1,
        GOAL=2
};

static struct vec2 advance_pos(struct vec2 pos, enum direction dir) {
        switch(dir) {
        case NORTH: pos.y--; break;
        case SOUTH: pos.y++; break;
        case WEST: pos.x--; break;
        case EAST: pos.x++; break;
        }
        return pos;
}

static enum status step(struct IntCodeMachine *machine, enum direction dir) {
        machine_send_input(machine, dir);
        machine_run(machine);
        long out;
        machine_recv_output(machine, &out);
        machine_run(machine);
        return out;
}

#ifdef DEBUG
static void show_map(int map[MAPSIZE][MAPSIZE], struct vec2 pos) {
        fprintf(stderr, " ");
        for (size_t i=0; i<MAPSIZE; i++) {
                fprintf(stderr, "--");
        }
        fprintf(stderr, "\n");
        
        for (size_t j=0; j<MAPSIZE; j++) {
                fprintf(stderr, "|");
                for (size_t i=0; i<MAPSIZE; i++) {
                        char c;
                        if (pos.x == (int)i && pos.y == (int)j) {
                                c = 'D';
                        } else {
                                if (map[j][i] < 0)
                                        c = '#';
                                else if (map[j][i] == 0)
                                        c = ' ';
                                else if (map[j][i] == 1)
                                        c = '.';
                                else if (map[j][i] == 2)
                                        c = ',';
                                else if (map[j][i] == 3)
                                        c = ':';
                                else if (map[j][i] == 4)
                                        c = ';';
                                else if (map[j][i] <= 9)
                                        c = map[j][i] + '0';
                                else if (map[j][i] <= 'Z'-'A'+10)
                                        c = map[j][i] - 10 + 'A';
                                else if (map[j][i] <= 'z'-'a'+'Z'-'A'+1+10)
                                        c = map[j][i] - ('Z'-'A'+1) - 10 + 'a';
                                else if (map[j][i] == GOALTOKEN)
                                        c = 'X';
                                else
                                        c = '-';
                        }
                        fprintf(stderr, "%c ", c);
                }
                fprintf(stderr, "|\n");
        }
        
        fprintf(stderr, " ");
        for (size_t i=0; i<MAPSIZE; i++) {
                fprintf(stderr, "--");
        }
}
#endif

static void draw_map(const struct vec2 pos, int map[MAPSIZE][MAPSIZE], struct IntCodeMachine *const machine, const struct vec2 start, const bool first) {
#ifdef DEBUG
        //show_map(map, pos);
        //usleep(50000);
#endif

        if (!first && pos.x == start.x && pos.y == start.y) {
#ifdef DEBUG
                show_map(map, pos);
#endif
                return;
        }

        enum direction best_d = NORTH;
        struct vec2 best_next = pos;
        int mincolor = INT_MAX;
        for (enum direction d = NORTH; d <= EAST; d++) {
                struct vec2 next = advance_pos(pos, d);
                if (INDEX(map,next) >= 0 && INDEX(map,next) < mincolor) {
                        mincolor = INDEX(map,next);
                        best_d = d;
                        best_next = next;
                }
        }

        if (mincolor == INT_MAX) {
                FAIL("Are we trapped between walls or something?");
        }

        enum status s = step(machine, best_d);
        switch(s) {
        case WALL:
                INDEX(map, best_next) = -1;
                return draw_map(pos, map, machine, start, false);
        case FREE:
                INDEX(map, best_next)++;
                return draw_map(best_next, map, machine, start, false);
        case GOAL:
                INDEX(map, best_next) = GOALTOKEN;
                return draw_map(best_next, map, machine, start, false);
        default:
                FAIL("Invalid state: %d", s);
        }
}

static int search_map(const struct vec2 pos, int map[MAPSIZE][MAPSIZE]) {
        int curr = INDEX(map,pos);

        for (enum direction d = NORTH; d<=EAST; d++) {
                struct vec2 nextpos = advance_pos(pos, d);
                if (INDEX(map, nextpos) == 0) {
                        INDEX(map, nextpos) = curr+1;
                        int r = search_map(nextpos, map);
                        if (r != -1)
                                return r;
                } else if (INDEX(map, nextpos) == GOALTOKEN) {
                        return curr+1;
                }
        }

        return -1;
}

static void get_map(const char *const input, int map[MAPSIZE][MAPSIZE], const struct vec2 start) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);

        machine_run(&machine);
        draw_map(start, map, &machine, start, true);

        // reset map
        for (size_t j=0; j<MAPSIZE; j++) {
                for (size_t i=0; i<MAPSIZE; i++) {
                        if (map[j][i] > 0 && map[j][i] != GOALTOKEN)
                                map[j][i] = 0;
                }
        }
}

struct queue {
        struct vec3 elements[QUEUESIZE];
        size_t start, end;
};

static void queue_init(struct queue *q) {
        q->start = 0;
        q->end = 0;
}

static bool queue_empty(struct queue *q) {
        return q->start == q->end;
}

static void queue_enqueue(struct queue *q, struct vec3 v) {
        q->elements[q->start] = v;
        q->start = (q->start + 1) % QUEUESIZE;
}

static struct vec3 queue_dequeue(struct queue *q) {
        struct vec3 r = q->elements[q->end];
        q->end = (q->end + 1) % QUEUESIZE;
        return r;
}

static int flood_map(const struct vec2 start2, int map[MAPSIZE][MAPSIZE]) {
        struct queue qq, *q=&qq;
        queue_init(q);

        struct vec3 start = {.x=start2.x, .y=start2.y, .z=1};

        int res = -1;
        queue_enqueue(q, start);
        while (!queue_empty(q)) {
                struct vec3 pos = queue_dequeue(q);
                struct vec2 pos2 = {.x=pos.x, .y=pos.y};
                INDEX(map,pos) = res = pos.z;
                
#ifdef DEBUG
                //show_map(map, pos2);
                //usleep(50000);
#endif

                for (enum direction d=NORTH; d<=EAST; d++) {
                        struct vec2 next2 = advance_pos(pos2, d);
                        struct vec3 next = {.x=next2.x, .y=next2.y, .z=pos.z+1};
                        if (INDEX(map, next) == 0) {
                                queue_enqueue(q, next);
                        }
                }
        }

        return res-1;
}

static void solution1(const char *const input, char *const output) {
        static int map[MAPSIZE][MAPSIZE];
        const struct vec2 start = {.x=MAPSIZE/2, .y=MAPSIZE/2};
        
        get_map(input, map, start);
        int result = search_map(start, map);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *const input, char *const output) {
        static int map[MAPSIZE][MAPSIZE];
        const struct vec2 start = {.x=MAPSIZE/2, .y=MAPSIZE/2};
        
        get_map(input, map, start);

        struct vec2 end;
        for (size_t j=0; j<MAPSIZE; j++) {
                for (size_t i=0; i<MAPSIZE; i++) {
                        if (map[j][i] == GOALTOKEN) {
                                end.x = i;
                                end.y = j;
                        }
                }
        }

        int result = flood_map(end, map);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
