#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

#define MAZESIZE 81
#define QUEUESIZE (1<<8)
#define TOTALKEYS ('z'-'a'+1) // keys a through z
#define INITIALPOSITION TOTALKEYS // encode the initial position as an extra key
#define INITIALPOSITION1 TOTALKEYS // part 2, encode the initial position as an extra key
#define INITIALPOSITION2 TOTALKEYS+1 // part 2, encode the initial position as an extra key
#define INITIALPOSITION3 TOTALKEYS+2 // part 2, encode the initial position as an extra key
#define INITIALPOSITION4 TOTALKEYS+3 // part 2, encode the initial position as an extra key
#define TOTALPOSITIONS (TOTALKEYS+1) // keys a through z plus the initial position
#define TOTALPOSITIONS2 (TOTALKEYS+4) // keys a through z plus the initial position of each robot
#define CACHESIZE TOTALPOSITIONS2 * (1<<TOTALKEYS)
#define STARTTOKEN '@'

static int cache[CACHESIZE];

static int get_cache(unsigned position, unsigned acquired_keys) {
        // position: a number from 0 to TOTALPOSITIONS
        ASSERT(position < TOTALPOSITIONS, "incorrect position");

        // acquired_keys is a bitfield of as many bits as keys
        ASSERT(acquired_keys < 1<<TOTALKEYS, "incorrect acquired keys");

        // this means we need a cache of size TOTALPOSITIONS *
        // 1<<TOTALKEYS, this is less than 2GB, reasonable enough

        int n = cache[acquired_keys*TOTALPOSITIONS+position];
        if (n == 0) {
                return -1;
        } else if (n == -1) {
                return 0;
        } else {
                return n;
        }
}

static void set_cache(unsigned position, unsigned acquired_keys, unsigned distance) {
        ASSERT(position < TOTALPOSITIONS, "incorrect position");
        ASSERT(acquired_keys < 1<<TOTALKEYS, "incorrect acquired keys");
        ASSERT(distance <= INT_MAX, "distance too large");

        if (distance == 0) {
                cache[acquired_keys*TOTALPOSITIONS+position] = -1;
        } else {
                cache[acquired_keys*TOTALPOSITIONS+position] = distance;
        }
}

static bool get_precalc_reachable(unsigned reachable[TOTALPOSITIONS][TOTALPOSITIONS],
                                  unsigned from, unsigned to, unsigned keys) {
        unsigned needed = reachable[from][to];
        for (int key=0; key<TOTALKEYS; key++) {
                if (needed & 1<<key && !(keys & 1<<key)) {
                        return false;
                }
        }
        return true;
}

/*
static bool get_precalc_reachable2(unsigned reachable[TOTALPOSITIONS2][TOTALPOSITIONS2],
                                  unsigned from, unsigned to, unsigned keys) {
        unsigned needed = reachable[from][to];
        for (int key=0; key<TOTALKEYS; key++) {
                if (needed & 1<<key && !(keys & 1<<key)) {
                        return false;
                }
        }
        return true;
}
*/

static unsigned get_precalc_distance(unsigned distances[TOTALPOSITIONS][TOTALPOSITIONS],
                                     unsigned from, unsigned to) {
        return distances[from][to];
}

/*
static unsigned get_precalc_distance2(unsigned distances[TOTALPOSITIONS2][TOTALPOSITIONS2],
                                     unsigned from, unsigned to) {
        return distances[from][to];
}
*/

static unsigned best_distance_to_get_keys(unsigned distances[TOTALPOSITIONS][TOTALPOSITIONS],
                                          unsigned reachable[TOTALPOSITIONS][TOTALPOSITIONS],
                                          unsigned position, unsigned acquired_keys,
                                          unsigned num_acquired_keys) {
        if (num_acquired_keys == TOTALKEYS) {
                return 0;
        }

        int dist_cache = get_cache(position, acquired_keys);
        if (dist_cache > -1) {
                return (unsigned)dist_cache;
        }

        unsigned result = INT_MAX;
        for (unsigned i=0; i<TOTALKEYS; i++) {
                if (!(acquired_keys & 1<<i) &&
                    get_precalc_reachable(reachable, position, i, acquired_keys)) {
                        unsigned distance  = get_precalc_distance(distances, position, i);
                        
                        distance += best_distance_to_get_keys(distances, reachable, i,
                                                              acquired_keys | 1<<i, num_acquired_keys + 1);
                        
                        if (distance < result) {
                                result = distance;
                        }
                }
        }

        set_cache(position, acquired_keys, result);
        return result;
}

struct vec4 {
        unsigned x,y,z,w;
};

struct vec2 {
        unsigned x,y;
};

struct queue {
        size_t start, end;
        struct vec4 elements[QUEUESIZE];
};

static void queue_init(struct queue *q) {
        q->start = 0;
        q->end = 0;
}

static void queue_enqueue(struct queue *q, struct vec4 e) {
        q->elements[q->end] = e;
        q->end = (q->end + 1) % QUEUESIZE;
        ASSERT(q->end != q->start, "queue full");
}

static bool queue_empty(struct queue *q) {
        return q->start == q->end;
}

static struct vec4 queue_dequeue(struct queue *q) {
        ASSERT(q->end != q->start, "queue empty");
        struct vec4 e = q->elements[q->start];
        q->start = (q->start + 1) % QUEUESIZE;
        return e;
}

static struct vec4 advance(struct vec4 pos, unsigned dir) {
        if (dir == 0) {
                pos.y--;
        } else if (dir == 1) {
                pos.x++;
        } else if (dir == 2) {
                pos.y++;
        } else if (dir == 3) {
                pos.x--;
        } else {
                FAIL("impossiburu");
        }

        return pos;
}

static void bfs(char maze[MAZESIZE][MAZESIZE],
                struct vec2 p1, struct vec2 p2,
                unsigned *distance, unsigned *keys) {
        struct vec4 start = {.x=p1.x, .y=p1.y, .z=0, .w=0};
        struct vec4 goal = {.x=p2.x, .y=p2.y, .z=0, .w=0};

        static bool visited[MAZESIZE][MAZESIZE];
        memset(visited, 0, MAZESIZE*MAZESIZE*sizeof(bool));

        static struct queue qq, *q=&qq;
        queue_init(q);

        queue_enqueue(q, start);
        while (!queue_empty(q)) {
                struct vec4 pos = queue_dequeue(q);
                if (pos.x == goal.x && pos.y == goal.y) {
                        *distance = pos.z;
                        *keys = pos.w;
                        return;
                }

                visited[pos.y][pos.x] = true;

                for (unsigned dir=0; dir<4; dir++) {
                        struct vec4 next = advance(pos, dir);
                        char c = maze[next.y][next.x];
                        if (c != '#' && !visited[next.y][next.x]) {
                                next.z++;
                                if (isupper(c)) {
                                        next.w |= 1<<(c-'A');
                                }
                                queue_enqueue(q, next);
                        }
                }
        }

        *distance = INT_MAX;
        *keys = (1<<TOTALKEYS)-1;
}

static void precalculate(char maze[MAZESIZE][MAZESIZE],
                         unsigned distances[TOTALPOSITIONS][TOTALPOSITIONS],
                         unsigned reachable[TOTALPOSITIONS][TOTALPOSITIONS],
                         struct vec2 initial_position, struct vec2 key_positions[TOTALKEYS]) {
        // compute a 2D matrix with the distance from each important
        // position to each other important position

        // compute a 2D matrix with the set of keys (expressed as a
        // bitfield) necessary to reach an important position from
        // another important position
        
        for (unsigned k1=0; k1<TOTALKEYS; k1++) {
                for (unsigned k2=0; k2<TOTALKEYS; k2++) {
                        unsigned distance, keys;
                        bfs(maze, key_positions[k1], key_positions[k2], &distance, &keys);
                        distances[k1][k2] = distance;
                        distances[k2][k1] = distance;
                        reachable[k1][k2] = keys;
                        reachable[k2][k1] = keys;
                }
        }
        for (unsigned k=0; k<TOTALKEYS; k++) {
                unsigned distance, keys;
                bfs(maze, initial_position, key_positions[k], &distance, &keys);
                distances[INITIALPOSITION][k] = distance;
                distances[k][INITIALPOSITION] = distance;
                reachable[INITIALPOSITION][k] = keys;
                reachable[k][INITIALPOSITION] = keys;
        }
        distances[INITIALPOSITION][INITIALPOSITION] = 0;
        reachable[INITIALPOSITION][INITIALPOSITION] = 0;
}

static void find_positions(char maze[MAZESIZE][MAZESIZE],
                           struct vec2 *initial_position, struct vec2 key_positions[TOTALKEYS]) {
        for (unsigned y=0; y<MAZESIZE; y++) {
                for (unsigned x=0; x<MAZESIZE; x++) {
                        char c = maze[y][x];
                        if (c == '@') {
                                initial_position->x = x;
                                initial_position->y = y;
                        } else if (islower(c)) {
                                key_positions[c-'a'].x = x;
                                key_positions[c-'a'].y = y;
                        }
                }
        }
}

static void parse_maze(const char *const input, char maze[MAZESIZE][MAZESIZE]) {
        unsigned x=0, y=0;

        for (unsigned i=0;; i++) {
                if (input[i] == '\n') {
                        if (input[i+1] == '\0') {
                                x--;
                                break;
                        }
                        
                        y++;
                        x=0;
                } else {
                        maze[y][x] = input[i];
                        x++;
                }
        }
        ASSERT(x == MAZESIZE-1 && y == MAZESIZE-1, "parsing error %u,%u",x,y);
}

static void adjust_part2(char maze[MAZESIZE][MAZESIZE],
                         struct vec2 initial_position, struct vec2 robot_positions[4]) {
        maze[initial_position.y][initial_position.x] = '#';
        maze[initial_position.y+1][initial_position.x] = '#';
        maze[initial_position.y-1][initial_position.x] = '#';
        maze[initial_position.y][initial_position.x+1] = '#';
        maze[initial_position.y][initial_position.x-1] = '#';
        
        robot_positions[0] = initial_position;
        robot_positions[0].x++; robot_positions[0].y++;
        robot_positions[1] = initial_position;
        robot_positions[1].x++; robot_positions[1].y--;
        robot_positions[2] = initial_position;
        robot_positions[2].x--; robot_positions[2].y++;
        robot_positions[3] = initial_position;
        robot_positions[3].x--; robot_positions[3].y--;
        
        maze[robot_positions[0].y][robot_positions[0].x] = '@';
        maze[robot_positions[1].y][robot_positions[1].x] = '@';
        maze[robot_positions[2].y][robot_positions[2].x] = '@';
        maze[robot_positions[3].y][robot_positions[3].x] = '@';
}

static void solution1(const char *const input, char *const output) {
        DBG("Parsing maze...");
        static char maze[MAZESIZE][MAZESIZE];
        parse_maze(input, maze);

        DBG("Finding important positions...");
        struct vec2 initial_position, key_positions[TOTALKEYS];
        find_positions(maze, &initial_position, key_positions);

        DBG("Precalculating distances and reachability...");
        static unsigned distances[TOTALPOSITIONS][TOTALPOSITIONS];
        static unsigned reachable[TOTALPOSITIONS][TOTALPOSITIONS];
        precalculate(maze, distances, reachable, initial_position, key_positions);

        DBG("Runing main recusive algorithm...");
        unsigned result = best_distance_to_get_keys(distances, reachable, INITIALPOSITION, 0, 0);

        DBG("Done.");
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

static void solution2(const char *const input, char *const output) {
        DBG("Parsing maze...");
        static char maze[MAZESIZE][MAZESIZE];
        parse_maze(input, maze);

        DBG("Finding important positions...");
        struct vec2 initial_position, key_positions[TOTALKEYS];
        find_positions(maze, &initial_position, key_positions);

        DBG("Adjusting for part 2...");
        struct vec2 robot_positions[4];
        adjust_part2(maze, initial_position, robot_positions);

        DBG("Precalculating distances and reachability for part 2...");
        //static unsigned distances[TOTALPOSITIONS2][TOTALPOSITIONS2];
        //static unsigned reachable[TOTALPOSITIONS2][TOTALPOSITIONS2];
        //precalculate2(maze, distances, reachable, robot_positions, key_positions);

        DBG("Runing main recusive algorithm...");
        //unsigned robot_encoded_positions[4] = { INITIALPOSITION1, INITIALPOSITION2,
        //                                        INITIALPOSITION3, INITIALPOSITION4 };
        //unsigned result = best_distance_to_get_keys2(distances, reachable, robot_encoded_positions, 0, 0);

        DBG("Done.");
        //snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
        (void)output;
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
