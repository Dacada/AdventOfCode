#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)>(b)?(b):(a))

struct line {
        bool x_axis;
        int value;
};

struct segment {
        int start;
        int end;
};

static int segment_cmp(const void *a, const void *b) {
        const struct segment *aa = a;
        const struct segment *bb = b;
        return aa->start - bb->start;
}

static int segment_length(struct segment x) {
        return x.end - x.start;
}

static bool segment_contains(struct segment x, int p) {
        return p >= x.start && p <= x.end;
}

struct line_segment {
        struct line line;
        struct segment segment;
};

struct line_segment_collection {
        struct line line;
        struct segment *segments;
        int len;
        int cap;
};

static void line_segment_collection_init(struct line_segment_collection *c, struct line l) {
        c->len = 0;
        c->cap = 4;
        c->segments = malloc(sizeof(*c->segments)*c->cap);
        c->line = l;
}

static void line_segment_collection_print(const struct line_segment_collection *c) {
        if (c->len == 0) {
                fputs("<empty>\n\n", stderr);
                return;
        }

        for (int i=0; i<c->len; i++) {
                fprintf(stderr, "%d to %d\n", c->segments[i].start, c->segments[i].end);
        }
        fputc('\n', stderr);
}

static void line_segment_collection_add(struct line_segment_collection *c, const struct line_segment *s) {
        ASSERT(s->line.value == s->line.value && c->line.x_axis == s->line.x_axis, "cannot add segment of different line");
        if (c->len >= c->cap) {
                c->cap *= 2;
                c->segments = realloc(c->segments, sizeof(*c->segments)*c->cap);
        }
        c->segments[c->len++] = s->segment;
}

static void line_segment_collection_free(struct line_segment_collection *c) {
        free(c->segments);
}

struct point {
        int x;
        int y;
};

static int distance_points(struct point a, struct point b) {
        return abs(a.x - b.x) + abs(a.y - b.y);
}

struct diagonal {
        struct point a;
        struct point b;
};

struct circle {
        struct point center;
        struct point point;
};

static int radius(const struct circle *c) {
        return distance_points(c->center, c->point);
}

static int distance_line_point(const struct point p, const struct line l) {
        int pval;
        if (l.x_axis) {
                pval = p.x;
        } else {
                pval = p.y;
        }
        return abs(l.value - pval);
}

static bool intersect_circle(const struct circle *circle, struct line line, struct line_segment *segment) {
        segment->line = line;
        
        int r = radius(circle);
        int dist_to_line = distance_line_point(circle->center, line);
        
        int dist_left = r - dist_to_line;
        if (dist_left < 0) { // circle does not reach line
                return false;
        }

        segment->segment.start = circle->center.x - dist_left;
        segment->segment.end = circle->center.x + dist_left;
        return true;
}

static inline void skip_newlines(const char **input) {
        while (**input == '\n') {
                *input += 1;
        }
}

static int parse_int(const char **input) {
        bool neg = **input == '-';
        if (neg) {
                *input += 1;
        }
        
        ASSERT(isdigit(**input), "parse error");
        int r = 0;
        while (isdigit(**input)) {
                r *= 10;
                r += **input - '0';
                *input += 1;
        }

        if (neg) {
                r = -r;
        }
        
        return r;
}

#define ASSERT_STR(input, str)                                          \
        ASSERT(strncmp(input, str, strlen(str)) == 0, "parse error");   \
        input += strlen(str);

static void parse_line(const char **input, struct circle *circle) {
        ASSERT_STR(*input, "Sensor at x=");
        circle->center.x = parse_int(input);
        ASSERT_STR(*input, ", y=");
        circle->center.y = parse_int(input);
        ASSERT_STR(*input, ": closest beacon is at x=");
        circle->point.x = parse_int(input);
        ASSERT_STR(*input, ", y=");
        circle->point.y = parse_int(input);
        ASSERT(**input == '\n' || **input == '\0', "parse error");
}

#undef ASSERT_STR

static int parse_input(const char *input, struct circle **circles) {
        int len = 0;
        int cap = 8;
        *circles = malloc(sizeof(**circles)*cap);
        
        while (*input != '\0') {
                if (len >= cap) {
                        cap *= 2;
                        *circles = realloc(*circles, sizeof(**circles)*cap);
                }
                parse_line(&input, &(*circles)[len++]);
                skip_newlines(&input);
        }

        return len;
}

static void solution1(const char *const input, char *const output) {
        const struct line line = { .x_axis=false, .value=2000000 };
        
        struct circle* circles;
        int len = parse_input(input, &circles);

        struct line_segment_collection segments;
        line_segment_collection_init(&segments, line);
        
        for (int i=0; i<len; i++) {
                struct circle *circle = circles + i;
                struct line_segment segment;
                if (intersect_circle(circle, line, &segment)) {
                    line_segment_collection_add(&segments, &segment);
                }
                line_segment_collection_print(&segments);
        }

        int total = 0;
        qsort(segments.segments, segments.len, sizeof(*segments.segments), segment_cmp);
        line_segment_collection_print(&segments);
        struct segment current = segments.segments[0];
        for (int i=1; i<segments.len; i++) {
                struct segment next = segments.segments[i];
                if (segment_contains(current, next.start)) {
                        DBG("next (%d,%d) starts in current", next.start, next.end);
                        if (!segment_contains(current, next.end)) {
                                current.end = next.end;
                                DBG("update current to (%d,%d)", current.start, current.end);
                        }
                } else {
                        DBG("next (%d,%d) starts out of current, change current to next", next.start, next.end);
                        total += segment_length(current);
                        current = next;
                }
        }
        total += segment_length(current);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
        line_segment_collection_free(&segments);
        free(circles);
}

static void init_circle_boundary(const struct circle *c, struct point *p) {
        *p = c->center;
        int r = radius(c);
        p->y -= r + 1;
}

static bool iterate_circle_boundary(const struct circle *c, struct point *p) {
        int r = radius(c);
        if (p->x == c->center.x - 1 && p->y == c->center.y - r) {
                return false;
        }

        if (p->y < c->center.y && p->x >= c->center.x) {
                p->y++;
                p->x++;
        } else if (p->y >= c->center.y && p->x > c->center.x) {
                p->y++;
                p->x--;
        } else if (p->y > c->center.y && p->x <= c->center.x) {
                p->y--;
                p->x--;
        } else if (p->y <= c->center.y && p->x < c->center.x) {
                p->y--;
                p->x++;
        } else {
                FAIL("logic error");
        }
        
        return true;
}

static bool point_in_circle(const struct circle *circle, struct point p) {
        return distance_points(circle->center, p) <= radius(circle);
}

static void draw_coord(const struct circle *circle, struct point p) {
#ifdef DEBUG
        int r = radius(circle);
        for (int j=-5; j<25; j++) {
                for (int i=-5; i<25; i++) {
                        struct point q = {.x=i, .y=j};
                        char c;
                        if (q.x == p.x && q.y == p.y) {
                                c = 'o';
                        } else if (distance_points(q, circle->center) <= r) {
                                c = '#'; 
                        } else {
                                c = ' ';
                        }
                        fputc(c, stderr);
                }
                fputc('\n', stderr);
        }
        fputc('\n', stderr);
#else
        (void)circle;
        (void)p;
#endif
}

static void solution2(const char *const input, char *const output) {
        struct circle* circles;
        int len = parse_input(input, &circles);

        struct point p = {.x=0, .y=0};
        for (int j=0; j<len; j++) {
                struct circle *circle = &circles[j];
                init_circle_boundary(circle, &p);
                do {
                        if (p.x < 0 || p.x > 4000000 || p.y < 0 || p.y > 4000000)
                                continue;
                        draw_coord(circle, p);
                        bool all = true;
                        for (int i=0; i<len; i++) {
                                if (i == j) {
                                        continue;
                                }
                                if (point_in_circle(&circles[i], p)) {
                                        all = false;
                                        break;
                                }
                        }
                        if (all) {
                                goto end;
                        }
                } while (iterate_circle_boundary(circle, &p));
        }
end:

        DBG("%d,%d", p.x, p.y);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", (long)p.x*4000000+(long)p.y);
        free(circles);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
