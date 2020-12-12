#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define abs(x) ((x<0)?(-x):(x))

enum direction {
        north,
        east,
        south,
        west,
        left,
        right,
        forward
};

struct instruction {
        enum direction dir;
        unsigned amount;
};

struct position {
        long east;
        long south;
};

static unsigned parse_int(const char **input) {
        unsigned n = 0;
        char c;
        while (isdigit(c = **input)) {
                n = n * 10 + c - '0';
                *input += 1;
        }
        return n;
}

static struct instruction parse_instruction(const char **input) {
        struct instruction instr;
        switch (**input) {
        case 'N':
                instr.dir = north;
                break;
        case 'E':
                instr.dir = east;
                break;
        case 'S':
                instr.dir = south;
                break;
        case 'W':
                instr.dir = west;
                break;
        case 'L':
                instr.dir = left;
                break;
        case 'R':
                instr.dir = right;
                break;
        case 'F':
                instr.dir = forward;
                break;
        default:
                FAIL("parse error");
        }
        *input += 1;
        instr.amount = parse_int(input);
        return instr;
}

static struct instruction *parse(const char *input, size_t *const ret_size) {
        struct instruction *l = malloc(sizeof(*l)*32);
        size_t capacity = 32;
        size_t size = 0;

        for (; *input!='\0'; input++) {
                struct instruction instr = parse_instruction(&input);
                ASSERT(*input == '\n', "parse error");

                if (size >= capacity) {
                        capacity *= 2;
                        l = realloc(l, sizeof(*l)*capacity);
                }
                l[size] = instr;
                size++;
        }

        *ret_size = size;
        return l;
}

static enum direction rotate(enum direction current, enum direction rotation, unsigned amount) {
        ASSERT(rotation == left || rotation == right, "unexpected rotation");
        ASSERT(current == north || current == east || current == south || current == west, "unexpected direction");
        ASSERT(amount % 90 == 0, "unexpected rotation amount %u", amount);
        return (current + (rotation==left?-1:1)*(amount / 90)) % 4;
}

static void solution1(const char *const input, char *const output) {
        size_t nav_size;
        struct instruction *nav = parse(input, &nav_size);

        struct position pos = {
                .east = 0,
                .south = 0,
        };
        enum direction orientation = east;
        
        for (size_t i=0; i<nav_size; i++) {
                struct instruction instr = nav[i];
                
                switch (instr.dir) {
                case north:
                        pos.south -= instr.amount;
                        break;
                case east:
                        pos.east += instr.amount;
                        break;
                case south:
                        pos.south += instr.amount;
                        break;
                case west:
                        pos.east -= instr.amount;
                        break;
                case left:
                case right:
                        orientation = rotate(orientation, instr.dir, instr.amount);
                        break;
                case forward:
                        switch (orientation) {
                        case north:
                                pos.south -= instr.amount;
                                break;
                        case east:
                                pos.east += instr.amount;
                                break;
                        case south:
                                pos.south += instr.amount;
                                break;
                        case west:
                                pos.east -= instr.amount;
                                break;
                        case left:
                        case right:
                        case forward:
                                FAIL("unexpected orientation");
                                break;
                        }
                        break;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", abs(pos.east) + abs(pos.south));
        free(nav);
}

static struct position rotate_rel_once(struct position relpos, enum direction rotation) {
        ASSERT(rotation == left || rotation == right, "unexpected rotation");
        
        long tmp = relpos.east;
        relpos.east = relpos.south;
        relpos.south = tmp;
        
        if (rotation == left) {
                relpos.south = -relpos.south;
        } else {
                relpos.east = -relpos.east;
        }

        return relpos;
}

static struct position rotate_rel(struct position relpos, enum direction rotation, const unsigned amount) {
        ASSERT(amount % 90 == 0, "unexpected rotation amount %u", amount);
        for (unsigned i=0; i<amount/90; i++) {
                relpos = rotate_rel_once(relpos, rotation);
        }
        return relpos;
}

static void solution2(const char *const input, char *const output) {
        size_t nav_size;
        struct instruction *nav = parse(input, &nav_size);

        struct position ship = {
                .east = 0,
                .south = 0,
        };
        struct position waypoint = {
                .east = 10,
                .south = -1,
        };

        for (size_t i=0; i<nav_size; i++) {
                struct instruction instr = nav[i];
                
                switch (instr.dir) {
                case north:
                        waypoint.south -= instr.amount;
                        break;
                case east:
                        waypoint.east += instr.amount;
                        break;
                case south:
                        waypoint.south += instr.amount;
                        break;
                case west:
                        waypoint.east -= instr.amount;
                        break;
                case left:
                case right:
                        waypoint = rotate_rel(waypoint, instr.dir, instr.amount);
                        break;
                case forward:
                        ship.south += waypoint.south * instr.amount;
                        ship.east += waypoint.east * instr.amount;
                        break;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", abs(ship.east) + abs(ship.south));
        free(nav);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
