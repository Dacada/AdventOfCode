#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define NODESX 35
#define NODESY 29

struct node {
        unsigned size;
        unsigned used;
        unsigned avail;
        unsigned perc;
};

static void skip_line(const char **const input) {
        while (**input != '\n') {
                *input += 1;
        }
        *input += 1;
}

static void skip_whitespace(const char **const input) {
        while (isspace(**input)) {
                *input += 1;
        }
}

static unsigned parse_number(const char **const input) {
        unsigned n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }
        return n;
}

static unsigned parse_cell(const char **const input, char c) {
        skip_whitespace(input);
        unsigned n = parse_number(input);
        ASSERT(**input == c, "parse error");
        *input += 1;
        return n;
}

static void parse_line(const char **const input, struct node *const node, unsigned *const x, unsigned *const y) {
        while (**input != '-') {
                *input += 1;
        }
        *input += 1;

        ASSERT(**input == 'x', "parse error");
        *input += 1;
        *x = parse_number(input);
        
        ASSERT(**input == '-', "parse error");
        *input += 1;
        
        ASSERT(**input == 'y', "parse error");
        *input += 1;
        *y = parse_number(input);

        node->size = parse_cell(input, 'T');
        node->used = parse_cell(input, 'T');
        node->avail = parse_cell(input, 'T');
        node->perc = parse_cell(input, '%');
}

static void parse_input(const char *input, struct node grid[NODESY][NODESX]) {
        skip_line(&input);
        skip_line(&input);

        while (*input != '\0') {
                unsigned x, y;

                struct node node;
                parse_line(&input, &node, &x, &y);
                grid[y][x] = node;

                ASSERT(*input == '\n' || *input == '\0', "parse error");
                if (*input == '\n') {
                        input += 1;
                }
        }
}

#define ITERATE_NODE_PAIRS(grid)                                        \
        for (unsigned Aj=0; Aj<NODESY; Aj++) {                          \
        for (unsigned Ai=0; Ai<NODESX; Ai++) {                          \
        struct node *A = &grid[Aj][Ai];                                 \
        if (A->used == 0) {                                             \
                continue;                                               \
        }                                                               \
        for (unsigned Bj=0; Bj<NODESY; Bj++) {                          \
        for (unsigned Bi=0; Bi<NODESX; Bi++) {                          \
        if (Bi == Ai && Bj == Aj) {                                     \
                continue;                                               \
        }                                                               \
        struct node *B = &grid[Bj][Bi];                                 \
        if (A->used <= B->avail)

#define END_NODE_PAIRS                          \
        }                                       \
        }                                       \
        }                                       \
        }

static void solution1(const char *const input, char *const output) {
        struct node grid[NODESY][NODESX];
        parse_input(input, grid);

        unsigned count = 0;
        ITERATE_NODE_PAIRS(grid) {
                count++;
        } END_NODE_PAIRS;
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
}

static unsigned search(struct node grid[NODESY][NODESX], unsigned goal_x, unsigned goal_y) {
        // we're moving a hole around basically, pretty print the grid (node
        // with used=0 as '_', goal as '@', node with used>100 as '#') and you
        // can see one wall of # between _ and @, the goal here is to find the
        // length of the shortest path from _ to the left of @, and this code
        // is tailored for a very specific case of my input which might not
        // work for any input (but it's very easy either way and you can find
        // out by hand with the pretty print)
        
        unsigned x=0, y=0;
        for (unsigned j=0; j<NODESY; j++) {
                for (unsigned i=0; i<NODESX; i++) {
                        if (grid[j][i].used == 0) {
                                x = i;
                                y = j;
                        }
                }
        }

        unsigned steps = 0;
        while (grid[y][x].used < 100) {
                y--;
                steps++;
        }
        y++;
        steps--;
        while (grid[y-1][x].used > 100) {
                x--;
                steps++;
        }
        while (y != goal_y) {
                y--;
                steps++;
        }
        while (x != goal_x) {
                x++;
                steps++;
        }

        return steps;
}

static void solution2(const char *const input, char *const output) {
        struct node grid[NODESY][NODESX];
        parse_input(input, grid);

        unsigned moves = search(grid, NODESX-2, 0);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", moves + 1 + 5*(NODESX-2));
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
