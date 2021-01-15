#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define WIDTH 50
#define HEIGHT 6

enum instr {
        INSTR_RECT,
        INSTR_ROT_ROW,
        INSTR_ROT_COL,
        INSTR_LAST
};

struct instruction {
        enum instr instr;
        unsigned a, b;
};

static bool screen[WIDTH*HEIGHT];
static inline bool screen_get(const unsigned x, const unsigned y) {
        return screen[y*WIDTH+x];
}
static inline void screen_set(const unsigned x, const unsigned y, const bool val) {
        screen[y*WIDTH+x] = val;
}
static void print_screen(void) {
        for (unsigned j=0; j<HEIGHT; j++) {
                for (unsigned i=0; i<WIDTH; i++) {
                        if (screen_get(i, j)) {
                                printf("# ");
                        } else {
                                printf("  ");
                        }
                }
                printf("\n");
        }
        printf("\n");
}

static void run_instruction(const struct instruction *instr) {
        switch (instr->instr) {
        case INSTR_RECT:
                DBG("rect %ux%u", instr->a, instr->b);
                for (unsigned j=0; j<instr->b; j++) {
                        for (unsigned i=0; i<instr->a; i++) {
                                screen_set(i, j, true);
                        }
                }
                break;
                
        case INSTR_ROT_ROW:
                DBG("rotate column x=%u by %u", instr->a, instr->b);
        {
                unsigned j = instr->a;
                bool newrow[WIDTH];
                for (unsigned i=0; i<WIDTH; i++) {
                        newrow[(i+instr->b) % WIDTH] = screen_get(i, j);
                }
                for (unsigned i=0; i<WIDTH; i++) {
                        screen_set(i, j, newrow[i]);
                }
        }
        break;
        
        case INSTR_ROT_COL:
                DBG("rotate row y=%u by %u", instr->a, instr->b);
        {
                unsigned i = instr->a;
                bool newcol[HEIGHT];
                for (unsigned j=0; j<HEIGHT; j++) {
                        newcol[(j+instr->b) % HEIGHT] = screen_get(i, j);
                }
                for (unsigned j=0; j<HEIGHT; j++) {
                        screen_set(i, j, newcol[j]);
                }
        }
        break;
        
        case INSTR_LAST:
        default:
                FAIL("unexpected instruction");
        }

#ifdef DEBUG
        print_screen();
#endif
}

static unsigned parse_int(const char **const input) {
        char c;
        unsigned n = 0;
        while (isdigit(c = **input)) {
                n = n * 10 + c - '0';
                *input += 1;
        }
        return n;
}

static struct instruction parse_line(const char **const input) {
        struct instruction res;

        *input += 1;
        if (**input == 'e') {
                res.instr = INSTR_RECT;
                *input += 4;
                DBG("%s", *input);
                res.a = parse_int(input);
                ASSERT(res.a > 0, "parse error");
                ASSERT(**input == 'x', "parse error");
                *input += 1;
                res.b = parse_int(input);
                ASSERT(res.b > 0, "parse error");
        } else if (**input == 'o') {
                *input += 6;
                if (**input == 'c') {
                        *input += 7;
                        res.instr = INSTR_ROT_COL;
                        ASSERT(**input == 'x', "parse error");
                } else if (**input == 'r') {
                        *input += 4;
                        res.instr = INSTR_ROT_ROW;
                        ASSERT(**input == 'y', "parse error");
                } else {
                        FAIL("parse error");
                }
                *input += 2;
                res.a = parse_int(input);
                ASSERT(**input == ' ', "parse error");
                *input += 4;
                res.b = parse_int(input);
                ASSERT(res.b > 0, "parse error");
        } else {
                FAIL("parse error");
        }

        return res;
}

static struct instruction *parse(const char *input) {
        size_t capacity = 8;
        struct instruction *list = malloc(sizeof(*list)*capacity);
        size_t length = 0;

        while (*input != '\0') {
                list[length++] = parse_line(&input);
                if (*input == '\0') {
                        break;
                }
                ASSERT(*input == '\n', "parse error");
                input++;
                
                if (length >= capacity) {
                        capacity *=2;
                        list = realloc(list, sizeof(*list)*capacity);
                }
        }

        list[length].instr = INSTR_LAST;

        return list;
}

static void run(const char *const input) {
        struct instruction *instructions = parse(input);

        for (size_t i=0;; i++) {
                struct instruction *instruction = instructions + i;
                if (instruction->instr == INSTR_LAST) {
                        break;
                }

                run_instruction(instruction);
        }
        
        free(instructions);
}

static void solution1(const char *const input, char *const output) {
        run(input);
        
        unsigned count = 0;
        for (unsigned j=0; j<HEIGHT; j++) {
                for (unsigned i=0; i<WIDTH; i++) {
                        if (screen_get(i, j)) {
                                count++;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
}

static void solution2(const char *const input, char *const output) {
        run(input);
        print_screen();
        snprintf(output, OUTPUT_BUFFER_SIZE, "output printed");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
