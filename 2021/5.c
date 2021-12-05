#include <aoclib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)<(b))?(b):(a))

#define WIDTH (1<<10)
#define HEIGHT (1<<10)

static unsigned field[HEIGHT][WIDTH];

struct point {
        size_t x;
        size_t y;
};

struct line {
        struct point start;
        struct point end;
};

static size_t parse_number(const char **const input) {
        ASSERT(isdigit(**input), "parse error");
        size_t n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }
        return n;
}

static struct point parse_point(const char **const input) {
        struct point p;
        p.x = parse_number(input);
        ASSERT(**input == ',', "parse error");
        *input += 1;
        p.y = parse_number(input);
        return p;
}

static struct line parse_line(const char **const input) {
        struct line l;
        l.start = parse_point(input);

        ASSERT(strncmp(*input, " -> ", 4) == 0, "parse error");
        *input += 4;

        l.end = parse_point(input);

        ASSERT(**input == '\n' || **input == '\0', "parse error");
        while (**input == '\n') {
                *input += 1;
        }
        return l;
}

static struct line *parse_input(const char *input, size_t *const len) {
        size_t size = 16;
        *len = 0;
        struct line *list = malloc(size * sizeof(*list));

        while (*input != '\0') {
                if (*len >= size) {
                        size *= 2;
                        list = realloc(list, size * sizeof(*list));
                }
                list[*len] = parse_line(&input);
                *len += 1;
        }

        return list;
}

static void coverlines(const struct line *const lines, const size_t nlines, bool diagonals) {
        for (size_t i=0; i<nlines; i++) {
                if (lines[i].start.x == lines[i].end.x) {
                        size_t x = lines[i].start.x;
                        size_t miny = MIN(lines[i].start.y, lines[i].end.y);
                        size_t maxy = MAX(lines[i].start.y, lines[i].end.y);
                        for (size_t y=miny; y<=maxy; y++) {
                                field[y][x]++;
                        }
                } else if (lines[i].start.y == lines[i].end.y) {
                        size_t y = lines[i].start.y;
                        size_t minx = MIN(lines[i].start.x, lines[i].end.x);
                        size_t maxx = MAX(lines[i].start.x, lines[i].end.x);
                        for (size_t x=minx; x<=maxx; x++) {
                                field[y][x]++;
                        }
                } else if (diagonals) {
                        struct point a, b;
                        if (lines[i].start.x < lines[i].end.x) {
                                a = lines[i].start;
                                b = lines[i].end;
                        } else {
                                a = lines[i].end;
                                b = lines[i].start;
                        }

                        if (a.y > b.y) {
                                for (size_t x=a.x, y=a.y; x <= b.x && y >= b.y; x++, y--) {
                                        field[y][x]++;
                                        if (y == 0) {
                                                break;
                                        }
                                }
                        } else {
                                for (size_t x=a.x, y=a.y; x <= b.x && y <= b.y; x++, y++) {
                                        field[y][x]++;
                                }
                        }
                }
        }
}

static unsigned countcross(void) {        
        unsigned count = 0;
        for (size_t y=0; y<HEIGHT; y++) {
                for (size_t x=0; x<WIDTH; x++) {
                        if (field[y][x] > 1) {
                                count++;
                        }
                }
        }
        return count;
}

static void solution(const char *const input, char *const output, bool diagonals) {
        size_t nlines;
        struct line *lines = parse_input(input, &nlines);

        coverlines(lines, nlines, diagonals);

        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", countcross());
        free(lines);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, false);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, true);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
