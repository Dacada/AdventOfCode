#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SIDE_LEN 1024

struct point {
        int x;
        int y;
};

#define IDX(p) ((p).y*SIDE_LEN+(p).x)
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)<(b)?(b):(a))

static int simulate_drop_step(int *map, struct point *point, int bottom) {
        point->y++;
        if (point->y >= SIDE_LEN) {
                return 2;
        }
        if (map[IDX(*point)] == 0 && point->y < bottom) {
                return 0;
        }
        
        point->x--;
        ASSERT(point->x >= 0, "out of bounds");
        if (map[IDX(*point)] == 0 && point->y < bottom) {
                return 0;
        }

        point->x += 2;
        ASSERT(point->x < SIDE_LEN, "out of bounds");
        if (map[IDX(*point)] == 0 && point->y < bottom) {
                return 0;
        }

        point->x--;
        point->y--;
        return 1;
}

static bool simulate_drop(int *map, struct point point, int bottom) {
        if (map[IDX(point)] != 0) {
                return false;
        }
        for(;;) {
                int res = simulate_drop_step(map, &point, bottom);
                if (res == 1) {
                        map[IDX(point)] = 2;
                        return true;
                } else if (res == 2) {
                        return false;
                }
        }
}

static void make_line(int *map, struct point a, struct point b) {
        ASSERT(a.x >= 0 && a.x < SIDE_LEN &&
               a.y >= 0 && a.y < SIDE_LEN &&
               b.x >= 0 && b.x < SIDE_LEN &&
               b.y >= 0 && b.y < SIDE_LEN, "parse error");

        struct point p;
        int from, to;
        int *change;
        if (a.x == b.x) {
                p.x = a.x;
                from = MIN(a.y, b.y);
                to = MAX(a.y, b.y);
                change = &p.y;
        } else if (a.y == b.y) {
                p.y = a.y;
                from = MIN(a.x, b.x);
                to = MAX(a.x, b.x);
                change = &p.x;
        } else {
                FAIL("parse error");
        }
        
        for (int i=from; i<=to; i++) {
                *change = i;
                map[IDX(p)] = 1;
        }
}

static int parse_int(const char **input) {
        ASSERT(isdigit(**input), "parse error");
        int r = 0;
        while (isdigit(**input)) {
                r *= 10;
                r += **input - '0';
                *input += 1;
        }
        return r;
}

static struct point parse_point(const char **input) {
        struct point res;
        res.x = parse_int(input);
        ASSERT(**input == ',', "parse error");
        *input += 1;
        res.y = parse_int(input);
        return res;
}

#define ASSERT_STR(input, str)                                          \
        ASSERT(strncmp(input, str, strlen(str)) == 0, "parse error");   \
        input += strlen(str);

static int parse_line(const char **input, int *map) {
        int bottom = 0;
        struct point a = parse_point(input);
        if (a.y > bottom) {
                bottom = a.y;
        }
        ASSERT_STR(*input, " -> ");
        struct point b = parse_point(input);
        if (b.y > bottom) {
                bottom = b.y;
        }
        make_line(map, a, b);
        while (**input != '\n') {
                ASSERT_STR(*input, " -> ");
                a = b;
                b = parse_point(input);
                if (b.y > bottom) {
                        bottom = b.y;
                }
                make_line(map, a, b);
        }
        return bottom;
}

#undef ASSERT_STR

static inline void skip_newlines(const char **input) {
        while (**input == '\n') {
                *input += 1;
        }
}

static void parse_input(const char *input, int *map, int *bottom) {
        *bottom = 0;
        while (*input != '\0') {
                int n = parse_line(&input, map);
                if (n > *bottom) {
                        *bottom = n;
                }
                ASSERT(*input == '\n' || *input == '\0', "parse error");
                skip_newlines(&input);
        }
        *bottom += 2;
}

#ifdef DEBUG
static void print_map(const int *map, int fromy, int toy, int fromx, int tox) {
        for (int j=fromy; j<toy; j++) {
                for (int i=fromx; i<tox; i++) {
                        struct point p;
                        p.x = i;
                        p.y = j;
                        int n = map[IDX(p)];
                        char c;
                        if (n == 0) {
                                c = '.';
                        } else if (n == 1) {
                                c = '#';
                        } else if (n == 2) {
                                c = 'o';
                        } else {
                                c = '?';
                        }
                        fputc(c, stderr);
                }
                fputc('\n', stderr);
        }
}
#endif

static void solution(const char *const input, char *const output, bool use_bottom) {
        static int map[SIDE_LEN*SIDE_LEN];
        int bottom;
        parse_input(input, map, &bottom);
        if (!use_bottom) {
                bottom = SIDE_LEN;
        }

        struct point sand;
        sand.x = 500;
        sand.y = 0;

#ifdef DEBUG
        print_map(map, 0, bottom, 490, 510);
#endif
        int count = 0;
        while (simulate_drop(map, sand, bottom)) {
                count += 1;
        }
#ifdef DEBUG
        print_map(map, 0, bottom, 490, 510);
#endif
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
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
