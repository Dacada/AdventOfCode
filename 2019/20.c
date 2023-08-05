#ifdef DEBUG
#define _DEFAULT_SOURCE
#include <unistd.h>
#endif

#include <aoclib.h>
#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#define INDEX(cols,j,i) ((j)*cols+(i))
#define MAXQUEUESIZE (1<<12)

#define PORTALBASE ('Z'-'A'+1)
#define MAXPORTALS (PORTALBASE*PORTALBASE)
#define PORTALINDEX(c1, c2) (((c1)-'A')*PORTALBASE + ((c2)-'A'))
#define ISPORTAL(c) isupper(c)

struct vec2 {
        size_t x,y;
};

struct portal {
        bool has_entrance, has_exit;
        bool entrance_outer, exit_outer;
        struct vec2 entrance, exit;
};

struct queue_cell {
        struct vec2 pos;
        int steps;
        int level;
};

struct queue {
        struct queue_cell cells[MAXQUEUESIZE];
        size_t start;
        size_t next;
};

static void queue_init(struct queue *q) {
        q->start=0;
        q->next=0;
}
static void queue_enqueue(struct queue *q, struct queue_cell cell) {
        q->cells[q->next] = cell;
        q->next = (q->next + 1) % MAXQUEUESIZE;
}
static bool queue_empty(struct queue *q) {
        return q->start == q->next;
}
static struct queue_cell queue_dequeue(struct queue *q) {
        struct queue_cell *c = &q->cells[q->start];
        q->start = (q->start + 1) % MAXQUEUESIZE;
        return *c;
}

static void add_portal(struct portal portals[MAXPORTALS], char c1, char c2, size_t y, size_t x, bool outer) {
        if (ISPORTAL(c1) && ISPORTAL(c2)) {
                struct portal *p = &portals[PORTALINDEX(c1,c2)];
                if (!p->has_entrance) {
                        p->entrance.x = x;
                        p->entrance.y = y;
                        p->has_entrance = true;
                        p->entrance_outer = outer;
                } else {
                        ASSERT(!p->has_exit, "Found a new portal but already has entrance and exit assigned");
                        p->exit.x = x;
                        p->exit.y = y;
                        p->has_exit = true;
                        p->exit_outer = outer;
                }
        }
}

static char *parse_maze(const char *const input,
                       size_t *const rows, size_t *const columns,
                       struct portal portals[MAXPORTALS]) {
        for (*columns=0;; ++*columns) {
                if (input[*columns] == '\n') {
                        break;
                }
        }
        
        for (*rows=0;; *rows+=*columns+1) {
                if (input[*rows] == '\0') {
                        break;
                }
        }
        *rows /= (*columns+1);

        char *maze = malloc(*columns * *rows * sizeof *maze);
        if (maze == NULL) {
                perror("malloc");
                return NULL;
        }

        size_t x=0, y=0;
        for (size_t i=0;; i++) {
                char c = input[i];
                if (c == '\n') {
                        x = 0;
                        y++;
                } else if (c == '\0') {
                        break;
                } else {
                        maze[INDEX(*columns, y, x)] = c;
                        x++;
                }
        }

        // Portals on the upper edge
        for (size_t i=0; i<*columns; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, 0, i)], maze[INDEX(*columns, 1, i)],
                           1, i, true);
        }
        
        // Portals on the lower edge
        for (size_t i=0; i<*columns; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, *rows-2, i)], maze[INDEX(*columns, *rows-1, i)],
                           *rows-2, i, true);
        }
        
        // Portals on the left edge
        for (size_t i=0; i<*rows; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, i, 0)], maze[INDEX(*columns, i, 1)],
                           i, 1, true);
        }
        
        // Portals on the right edge
        for (size_t i=0; i<*rows; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, i, *columns-2)], maze[INDEX(*columns, i, *columns-1)],
                           i, *columns-2, true);
        }

        // Finding the whole: A task harder than I imagined
        
        size_t hx=0, hy=0;
        for (size_t j=2; j<*rows; j++) {
                bool inhole = false;
                bool hashole = false;
                for (size_t i=2; i<*columns; i++) {
                        char c = maze[INDEX(*columns, j, i)];
                        if (inhole) {
                                if (c == ' ') {
                                } else {
                                        hashole = true;
                                        break;
                                }
                        } else {
                                if (c == ' ') {
                                        hx = i;
                                        inhole = true;
                                } else {
                                }
                        }
                }
                if (hashole) {
                        hy = j;
                        break;
                }
        }

        size_t lhx=0;
        for (size_t i=hx; i<*columns; i++) {
                char c = maze[INDEX(*columns, hy, i)];
                if (c != ' ' && !ISPORTAL(c)) {
                        break;
                }
                lhx++;
        }
        size_t lhy=0;
        for (size_t i=hy; i<*columns; i++) {
                char c = maze[INDEX(*columns, i, hx)];
                if (c != ' ' && !ISPORTAL(c)) {
                        break;
                }
                lhy++;
        }
        
        // Portals on the inner-upper edge
        for (size_t i=hx; i<hx+lhx; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, hy, i)], maze[INDEX(*columns, hy+1, i)],
                           hy, i, false);
        }
        
        // Portals on the inner-lower edge
        for (size_t i=hx; i<hx+lhx; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, hy+lhy-2, i)], maze[INDEX(*columns, hy+lhy-1, i)],
                           hy+lhy-1, i, false);
        }
        
        // Portals on the inner-left edge
        for (size_t i=hy; i<hy+lhy; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, i, hx)], maze[INDEX(*columns, i, hx+1)],
                           i, hx, false);
        }
        
        // Portals on the inner-right edge
        for (size_t i=hy; i<hy+lhy; i++) {
                add_portal(portals,
                           maze[INDEX(*columns, i, hx+lhx-2)], maze[INDEX(*columns, i, hx+lhx-1)],
                           i, hx+lhx-1, false);
        }
        
        return maze;
}

__attribute__((pure))
static struct vec2 advance(struct vec2 pos, int dir) {
        if (dir == 0) {
                pos.y-=1;
                return pos;
        } else if (dir == 1) {
                pos.x+=1;
                return pos;
        } else if (dir == 2) {
                pos.y+=1;
                return pos;
        } else if (dir == 3) {
                pos.x-=1;
                return pos;
        } else {
                FAIL("Bad direction");
        }
}

static struct vec2 step_out_portal(struct vec2 pos, char *maze, size_t columns) {
        ASSERT(ISPORTAL(maze[INDEX(columns, pos.y, pos.x)]), "Not stepping out of portal? %c at %lu,%lu", maze[INDEX(columns, pos.y, pos.x)],pos.x,pos.y);
        for (int d=0; d<4; d++) {
                struct vec2 ret = advance(pos, d);
                if (maze[INDEX(columns, ret.y, ret.x)] == '.')
                        return ret;
        }
        FAIL("No open tiles to step out of portal?");
}

static bool is_visited_at_level(int level, uint64_t **visited_levels, size_t columns, size_t y, size_t x) {
        int i = level/64;
        int e = level%64;

        return visited_levels[i][INDEX(columns, y, x)] & 1UL<<e;
}

static void set_level_as_visited(int level, uint64_t **visited_levels, size_t columns, size_t y, size_t x) {
        int i = level/64;
        int e = level%64;

        visited_levels[i][INDEX(columns, y, x)] |= 1UL<<e;
}

static int shortest_path(char *maze, size_t columns,
                         struct portal portals[MAXPORTALS], struct vec2 from, struct vec2 to,
                         bool recursive, uint64_t **visited_levels) {
        ASSERT(ISPORTAL(maze[INDEX(columns, from.y, from.x)]), "Did not spawn in portal?");
        ASSERT(ISPORTAL(maze[INDEX(columns, to.y, to.x)]), "Not heading to a portal?");

        struct vec2 realfrom = step_out_portal(from, maze, columns);
        struct vec2 realto = step_out_portal(to, maze, columns);
        
        set_level_as_visited(0, visited_levels, columns, from.y, from.x);

        static struct queue qq;
        struct queue *q=&qq;

        queue_init(q);

        struct queue_cell cl = { .pos = realfrom, .steps = 0, .level = 0 };
        queue_enqueue(q, cl);
        while (!queue_empty(q)) {
                struct queue_cell cell = queue_dequeue(q);
                struct vec2 pos = cell.pos;
                int steps = cell.steps;
                int level = cell.level;
                
                set_level_as_visited(level, visited_levels, columns, pos.y, pos.x);

                //DBG("Visiting: %lu,%lu (%d steps) at level %d",pos.x,pos.y,steps,level);
                
                for (int d=0; d<4; d++) { // for each direction
                        struct vec2 next = advance(pos, d);
                        if (next.x == realto.x && next.y == realto.y) {
                                if (level == 0) {
                                        return steps+1;
                                } else {
                                        continue;
                                }
                        }

                        if (level > 0 && next.x == realfrom.x && next.y == realfrom.y) {
                                continue;
                        }

                        if (is_visited_at_level(level, visited_levels, columns, next.y, next.x)) {
                                continue;
                        }
                        
                        char c = maze[INDEX(columns,next.y,next.x)]; // find out what's there
                        if (c == '.') { // traversable ground
                                struct queue_cell nextcell = { .pos=next, .steps=steps+1, .level=level };
                                queue_enqueue(q, nextcell);
                        } else if (ISPORTAL(c)) { // a portal
                                char c1 = c;
                                char c2 = '.';
                                for (int e=0; e<4; e++) { // find out its full name
                                        struct vec2 p = advance(next, e);
                                        char b = maze[INDEX(columns,p.y,p.x)];
                                        if (ISPORTAL(b)) {
                                                c2 = b;
                                                break;
                                        }
                                }
                                ASSERT(c2 != '.', "Failed to get portal name");

                                if (d == 0 || d == 3) {
                                        char tmp = c1;
                                        c1 = c2;
                                        c2 = tmp;
                                }

                                struct portal *p = &portals[PORTALINDEX(c1, c2)]; // look it up
                                ASSERT(p->has_exit, "Portal is incomplete: %c%c", c1,c2);

                                struct queue_cell nextcell;
                                int nextlevel = level;
                                if (p->entrance.x == next.x && p->entrance.y == next.y) {
                                        if (!recursive ||
                                            (p->entrance_outer && level > 0) ||
                                            !p->entrance_outer) {
                                                if (recursive) {
                                                        if (p->entrance_outer) {
                                                                nextlevel -= 1;
                                                        } else {
                                                                nextlevel += 1;
                                                        }
                                                }
                                                ASSERT(nextlevel < 128, "Too many levels?");
                                                nextcell.pos = step_out_portal(p->exit, maze, columns);
                                                set_level_as_visited(nextlevel, visited_levels, columns, p->exit.y, p->exit.x);
                                        } else {
                                                continue;
                                        }
                                } else if (p->exit.x == next.x && p->exit.y == next.y) {
                                        if (!recursive ||
                                            (p->exit_outer && level > 0) ||
                                            !p->exit_outer) {
                                                if (recursive) {
                                                        if (p->exit_outer) {
                                                                nextlevel -= 1;
                                                        } else {
                                                                nextlevel += 1;
                                                        }
                                                }
                                                ASSERT(nextlevel < 128, "Too many levels?");
                                                nextcell.pos = step_out_portal(p->entrance, maze, columns);
                                                set_level_as_visited(nextlevel, visited_levels, columns, p->entrance.y, p->entrance.x);
                                        } else {
                                                continue;
                                        }
                                } else {
                                        FAIL("Portal we're on doesn't have our location as one of its ends");
                                }

                                if (!recursive) {
                                        nextlevel = level;
                                }
                                
                                set_level_as_visited(level, visited_levels, columns, next.y, next.x);
                                        
                                nextcell.steps = steps + 1;
                                nextcell.level = nextlevel;
                                queue_enqueue(q, nextcell); // enqueue
                        }
                }
        }

        FAIL("Did not find a path");
}

static void solution1(const char *const input, char *const output) {
        size_t rows, columns;
        static struct portal portals[MAXPORTALS];
        char *maze = parse_maze(input, &rows, &columns, portals);

        /*
#ifdef DEBUG
        fprintf(stderr, "  ");
        for (size_t i=0; i<columns; i++) {
                fprintf(stderr, "%.2lu", i);
        }
        fprintf(stderr, "\n");
        for (size_t j=0; j<rows; j++) {
                fprintf(stderr, "%.2lu ", j);
                for (size_t i=0; i<columns; i++) {
                        char n = maze[INDEX(columns, j, i)];
                        char c;
                        switch (n) {
                        case ' ':
                        case '#':
                        case '.':
                                c = n; break;
                        default:
                                c = n;
                                ASSERT(isupper(c), "Expected uppercase character but got %c(%d)",c,c);
                                break;
                        }
                        fprintf(stderr, "%c ", c);
                }
                fprintf(stderr, "\n");
        }
#endif
        */

#ifdef DEBUG
        for (size_t i=0; i<MAXPORTALS; i++) {
                struct portal *p = portals+i;
                if (p->has_entrance) {
                        char c1 = i/PORTALBASE + 'A';
                        char c2 = i%PORTALBASE + 'A';
                        if (p->has_exit) {
                                DBG("Full portal %c%c at %lu,%lu and %lu,%lu",c1,c2,p->entrance.x,p->entrance.y, p->exit.x,p->exit.y);
                        } else {
                                DBG("Half portal %c%c at %lu,%lu",c1,c2,p->entrance.x,p->entrance.y);
                        }
                }
        }
#endif

        uint64_t *visited = malloc(columns * rows * sizeof *visited);
        if (visited == NULL) {
                perror("malloc");
                return;
        }
        memset(visited, 0, columns * rows * sizeof *visited);
        int result = shortest_path(maze, columns, portals,
                      portals[PORTALINDEX('A','A')].entrance,
                                   portals[PORTALINDEX('Z', 'Z')].entrance,
                                   false, &visited);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
        free(maze);
        free(visited);
}

static void solution2(const char *const input, char *const output) {
        size_t rows, columns;
        static struct portal portals[MAXPORTALS];
        char *maze = parse_maze(input, &rows, &columns, portals);

        uint64_t *v1 = malloc(columns * rows * sizeof *v1);
        if (v1 == NULL) {
                perror("malloc");
                return;
        }
        memset(v1, 0, columns * rows * sizeof *v1);
        
        uint64_t *v2 = malloc(columns * rows * sizeof *v2);
        if (v2 == NULL) {
                perror("malloc");
                return;
        }
        memset(v2, 0, columns * rows * sizeof *v2);

        uint64_t *visited[2] = {v1,v2};
        
        int result = shortest_path(maze, columns, portals,
                      portals[PORTALINDEX('A','A')].entrance,
                                   portals[PORTALINDEX('Z', 'Z')].entrance,
                                   true, visited);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
        free(maze);
        free(v1);
        free(v2);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
