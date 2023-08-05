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
#define CACHESIZE (1<<26) // ideal size found through trial and error (marked improvement from 25 to 26)
#define STARTTOKEN '@'

struct cache_element{
        unsigned positions[4];
        unsigned acquired_keys;
        int distance;
};
static struct cache_element cache[CACHESIZE];

static size_t get_cache_index(unsigned position, unsigned acquired_keys) {
        size_t i = 23;
        i = i * 31 + position;
        i = i * 31 + acquired_keys;
        return i % CACHESIZE;
}

static size_t get_cache_index2(unsigned positions[4], unsigned acquired_keys) {
        size_t i = 23;
        for (int j=0;j<4;j++)
                i = i * 31 + positions[j];
        i = i * 31 + acquired_keys;
        return i % CACHESIZE;
}

static int get_cache(unsigned position, unsigned acquired_keys) {
        size_t i = get_cache_index(position, acquired_keys);
        struct cache_element e = cache[i];

        if (e.positions[0] != position || e.acquired_keys != acquired_keys) {
                return -1;
        } else if (e.distance == 0) {
                return -1;
        } else if (e.distance == -1) {
                return 0;
        } else {
                return e.distance;
        }
}

static int get_cache2(unsigned positions[4], unsigned acquired_keys) {
        size_t i = get_cache_index2(positions, acquired_keys);
        struct cache_element e = cache[i];

        if (e.positions[0] != positions[0] || e.positions[1] != positions[1] ||
            e.positions[2] != positions[2] || e.positions[3] != positions[3] ||
            e.acquired_keys != acquired_keys) {
                return -1;
        } else if (e.distance == 0) {
                return -1;
        } else if (e.distance == -1) {
                return 0;
        } else {
                return e.distance;
        }
}

static void set_cache(unsigned position, unsigned acquired_keys, unsigned distance) {
        size_t i = get_cache_index(position, acquired_keys);
        
        if (distance == 0) {
                cache[i].distance = -1;
        } else {
                cache[i].distance = distance;
        }

        cache[i].positions[0] = position;
        cache[i].acquired_keys = acquired_keys;
}

static void set_cache2(unsigned positions[4], unsigned acquired_keys, unsigned distance) {
        size_t i = get_cache_index2(positions, acquired_keys);
        
        if (distance == 0) {
                cache[i].distance = -1;
        } else {
                cache[i].distance = distance;
        }

        cache[i].positions[0] = positions[0];
        cache[i].positions[1] = positions[1];
        cache[i].positions[2] = positions[2];
        cache[i].positions[3] = positions[3];
        cache[i].acquired_keys = acquired_keys;
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

static unsigned get_precalc_distance(unsigned distances[TOTALPOSITIONS][TOTALPOSITIONS],
                                     unsigned from, unsigned to) {
        return distances[from][to];
}

static unsigned get_precalc_distance2(unsigned distances[TOTALPOSITIONS2][TOTALPOSITIONS2],
                                     unsigned from, unsigned to) {
        return distances[from][to];
}

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

static unsigned best_distance_to_get_keys2(unsigned distances[TOTALPOSITIONS2][TOTALPOSITIONS2],
                                           unsigned reachable[TOTALPOSITIONS2][TOTALPOSITIONS2],
                                           unsigned positions[4], unsigned acquired_keys,
                                           unsigned num_acquired_keys) {
        if (num_acquired_keys == TOTALKEYS) {
                return 0;
        }

        int dist_cache = get_cache2(positions, acquired_keys);
        if (dist_cache > -1) {
                return (unsigned)dist_cache;
        }

        unsigned result = INT_MAX;
        for (unsigned i=0; i<TOTALKEYS; i++) {
                for (unsigned j=0; j<4; j++) {
                        if (!(acquired_keys & 1<<i) &&
                            get_precalc_reachable2(reachable, positions[j], i, acquired_keys)) {
                                unsigned distance  = get_precalc_distance2(distances, positions[j], i);

                                unsigned new_positions[4];
                                for (unsigned k=0; k<4; k++) {
                                        new_positions[k] = positions[k];
                                }
                                new_positions[j] = i;
                                
                                distance += best_distance_to_get_keys2(
                                        distances, reachable, new_positions, acquired_keys | 1<<i,
                                        num_acquired_keys + 1);
                                
                                if (distance < result) {
                                        result = distance;
                                }
                        }
                }
        }

        set_cache2(positions, acquired_keys, result);
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

__attribute__((pure))
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

static void precalculate2(char maze[MAZESIZE][MAZESIZE],
                          unsigned distances[TOTALPOSITIONS2][TOTALPOSITIONS2],
                          unsigned reachable[TOTALPOSITIONS2][TOTALPOSITIONS2],
                          struct vec2 initial_positions[4], struct vec2 key_positions[TOTALKEYS]) {
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
                for (unsigned p=0; p<4; p++) {
                        unsigned distance, keys;
                        bfs(maze, initial_positions[p], key_positions[k], &distance, &keys);
                        distances[INITIALPOSITION+p][k] = distance;
                        distances[k][INITIALPOSITION+p] = distance;
                        reachable[INITIALPOSITION+p][k] = keys;
                        reachable[k][INITIALPOSITION+p] = keys;
                }
        }
        for (unsigned p1=0; p1<4; p1++) {
                for (unsigned p2=0; p2<4; p2++) {
                        if (p1 == p2) {
                                distances[INITIALPOSITION+p1][INITIALPOSITION+p2] = 0;
                                reachable[INITIALPOSITION+p2][INITIALPOSITION+p1] = 0;
                        } else {
                                distances[INITIALPOSITION+p1][INITIALPOSITION+p2] = INT_MAX;
                                reachable[INITIALPOSITION+p2][INITIALPOSITION+p1] = INT_MAX;
                        }
                }
        }
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
        struct vec2 initial_position={.x=0, .y=0}, key_positions[TOTALKEYS];
        find_positions(maze, &initial_position, key_positions);

        DBG("Adjusting for part 2...");
        struct vec2 robot_positions[4];
        adjust_part2(maze, initial_position, robot_positions);

        DBG("Precalculating distances and reachability for part 2...");
        static unsigned distances[TOTALPOSITIONS2][TOTALPOSITIONS2];
        static unsigned reachable[TOTALPOSITIONS2][TOTALPOSITIONS2];
        precalculate2(maze, distances, reachable, robot_positions, key_positions);

        DBG("Runing main recusive algorithm...");
        unsigned robot_encoded_positions[4] = { INITIALPOSITION1, INITIALPOSITION2,
                                                INITIALPOSITION3, INITIALPOSITION4 };
        unsigned result = best_distance_to_get_keys2(distances, reachable, robot_encoded_positions, 0, 0);

        DBG("Done.");
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
