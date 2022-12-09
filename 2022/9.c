#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_LOCATIONS 1024

#define SIGN(x) ((x)==0?0:((x)>0?1:-1))

enum direction {
        DIR_UP,
        DIR_DOWN,
        DIR_LEFT,
        DIR_RIGHT,
};

struct instruction {
        enum direction direction;
        int steps;
};

struct point {
        int x;
        int y;
};

static bool record_location(bool locations[], struct point point) {
        point.x += MAX_LOCATIONS / 2;
        point.y += MAX_LOCATIONS / 2;
        ASSERT(point.x < MAX_LOCATIONS, "grid too small");
        ASSERT(point.y < MAX_LOCATIONS, "grid too small");
        int idx = point.y * MAX_LOCATIONS + point.x;
        
        bool ret = !locations[idx];
        locations[idx] = true;

        DBG("%d,%d -> %d", point.x, point.y, ret);
        return ret;
}

static void apply_direction(const enum direction dir, struct point *head) {
        switch (dir) {
        case DIR_UP:
                head->y += 1;
                break;
        case DIR_DOWN:
                head->y -= 1;
                break;
        case DIR_LEFT:
                head->x -= 1;
                break;
        case DIR_RIGHT:
                head->x += 1;
                break;
        default:
                FAIL("invalid direction");
        }
}

static void follow_head(struct point *tail, const struct point *head) {
        int dx = head->x - tail->x;
        int dy = head->y - tail->y;
        if (dx <= 1 && dx >= -1 && dy <= 1 && dy >= -1)
                return;
        tail->x += SIGN(dx);
        tail->y += SIGN(dy);
}

static void dbg_head_tail_pos(struct point h, struct point t) {
#ifdef DEBUG
        for (int j=0; j<5; j++) {
                for (int i=0; i<6; i++) {
                        int x = i;
                        int y = 4 - j;
                        char c;
                        if (h.x == x && h.y == y) {
                                c = 'H';
                        } else if (t.x == x && t.y == y) {
                                c = 'T';
                        } else {
                                c = '.';
                        }
                        fputc(c, stderr);
                }
                fputc('\n', stderr);
        }
        fputc('\n', stderr);
#else
        (void)h;
        (void)t;
#endif
}

static int apply_instruction(const struct instruction *instr, struct point *head, struct point *tail, bool *locations) {
        int total = 0;
        for (int i=0; i<instr->steps; i++) {
                apply_direction(instr->direction, head);
                follow_head(tail, head);
                dbg_head_tail_pos(*head, *tail);
                total += record_location(locations, *tail);
        }
        return total;
}

static int apply_instruction_2(const struct instruction *instr, struct point points[10], bool *locations) {
        int total = 0;
        for (int i=0; i<instr->steps; i++) {
                apply_direction(instr->direction, &points[0]);
                for (int j=0; j<9; j++) {
                        follow_head(&points[j+1], &points[j]);
                }
                dbg_head_tail_pos(points[0], points[9]);
                total += record_location(locations, points[9]);
        }
        return total;
}

static int parse_int(const char **const input) {
        int r = 0;
        ASSERT(isdigit(**input), "parse error");
        while (isdigit(**input)) {
                r *= 10;
                r += **input - '0';
                *input += 1;
        }
        return r;
}

static void parse_line(const char **const input, struct instruction *const instr) {
        switch (**input) {
        case 'R':
                instr->direction = DIR_RIGHT;
                break;
        case 'U':
                instr->direction = DIR_UP;
                break;
        case 'L':
                instr->direction = DIR_LEFT;
                break;
        case 'D':
                instr->direction = DIR_DOWN;
                break;
        default:
                FAIL("parse error");
        }
        
        *input += 1;
        ASSERT(**input == ' ', "parse error");
        *input += 1;

        instr->steps = parse_int(input);
        while (**input == '\n') {
                *input += 1;
        }
}

static int parse_input(const char *input, struct instruction **const instructions) {
        int len = 0;
        int cap = 16;
        *instructions = malloc(cap * sizeof(**instructions));

        while (*input != '\0') {
                if (len >= cap) {
                        cap *= 2;
                        *instructions = realloc(*instructions, cap * sizeof(**instructions));
                }
                parse_line(&input, &(*instructions)[len++]);
        }

        return len;
}

static void solution1(const char *const input, char *const output) {
        struct instruction *instructions;
        int len = parse_input(input, &instructions);

        struct point H = {.x=0, .y=0};
        struct point T = H;
        dbg_head_tail_pos(H, T);

        static bool locations[MAX_LOCATIONS*MAX_LOCATIONS];

        int total = 0;
        total += record_location(locations, T);
        for (int i=0; i<len; i++) {
                total += apply_instruction(&instructions[i], &H, &T, locations);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
        free(instructions);
}

static void solution2(const char *const input, char *const output) {
        struct instruction *instructions;
        int len = parse_input(input, &instructions);

        static struct point points[10];
        dbg_head_tail_pos(points[0], points[9]);

        static bool locations[MAX_LOCATIONS*MAX_LOCATIONS];

        int total = 0;
        total += record_location(locations, points[9]);
        for (int i=0; i<len; i++) {
                total += apply_instruction_2(&instructions[i], points, locations);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
        free(instructions);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
