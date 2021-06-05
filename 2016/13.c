#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_X (1<<7)
#define MAX_Y (1<<7)
#define QUEUE_SIZE (1<<10)

size_t favorite;

struct coord {
        size_t x, y;
};

static bool isopen(struct coord c) {
        size_t n = c.x*c.x + 3*c.x + 2*c.x*c.y + c.y + c.y*c.y + favorite;
        int bits = __builtin_popcount(n);
        return bits % 2 == 0;
}

#ifdef DEBUG
static void print_maze(struct coord lim) {
        fprintf(stderr, "  ");
        for (size_t x=0; x<=lim.x; x++) {
                fprintf(stderr, "%lu", x);
        }
        fprintf(stderr, "\n");
        for (size_t y=0; y<=lim.y; y++) {
                fprintf(stderr, "%lu ", y);
                for (size_t x=0; x<=lim.x; x++) {
                        struct coord c = {x, y};
                        if (isopen(c)) {
                                fprintf(stderr, ".");
                        } else {
                                fprintf(stderr, "#");
                        }
                }
                fprintf(stderr, "\n");
        }
}
#else
static void print_maze(struct coord lim) {
        (void)lim;
}
#endif

static bool visit(struct coord c) {
        static bool visited[MAX_Y][MAX_X];
        bool res = visited[c.y][c.x];
        visited[c.y][c.x] = true;
        return res;
}

static size_t search(struct coord from, struct coord to,
                     bool count_mode, size_t count_step) {
        struct cell {
                struct coord coord;
                size_t step;
        };
        
        struct cell *queue = malloc(sizeof(*queue)*QUEUE_SIZE);
        queue[0].coord = from;
        queue[0].step = 0;
        size_t qhead = 1;
        size_t qtail = 0;

        visit(from);

        size_t count = 1;
        while (qhead != qtail) {
                struct cell current = queue[qtail++];
                qtail %= QUEUE_SIZE;

                size_t step = current.step + 1;

                struct coord dirs[4];
                for (int i=0; i<4; i++) {
                        dirs[i] = current.coord;
                }
                dirs[0].x++;
                if (dirs[1].x > 0) {
                        dirs[1].x--;
                }
                dirs[2].y++;
                if (dirs[3].y > 0) {
                        dirs[3].y--;
                }
                
                for (int i=0; i<4; i++) {
                        if (!count_mode && dirs[i].x == to.x && dirs[i].y == to.y) {
                                free(queue);
                                return step;
                        }
                        if (isopen(dirs[i]) && !visit(dirs[i])) {
                                count++;
                                if (count_mode && step >= count_step) {
                                        continue;
                                }
                                queue[qhead].coord = dirs[i];
                                queue[qhead].step = step;
                                qhead++;
                                qhead %= QUEUE_SIZE;
                                ASSERT(qhead != qtail, "queue full");
                        }
                }
        }

        free(queue);
        return count;
}

static void parse(const char *input) {
        favorite = 0;
        while (isdigit(*input)) {
                favorite *= 10;
                favorite += *input - '0';
                input++;
        }
}

static void solution1(const char *const input, char *const output) {
        parse(input);
        
        struct coord limit = {9,6};
        print_maze(limit);

        struct coord from = {1,1};
        struct coord to = {31,39};
        size_t steps = search(from, to, false, 0);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", steps);
}

static void solution2(const char *const input, char *const output) {
        parse(input);
        
        struct coord from = {1,1};
        size_t count = search(from, from, true, 50);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", count);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
