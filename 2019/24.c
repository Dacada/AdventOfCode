#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define LAYOUTSIDE 5
#define LAYOUTSIZE (LAYOUTSIDE*LAYOUTSIDE)
#define RECURSIONSIZE (1<<8)

#define GETMASK(x,y) (1<<((y)*LAYOUTSIDE+(x)))
#define INDEXLAYOUT(x,y,l) ((bool)((l) & GETMASK(x,y)))

static void print(unsigned layout) {
        for (int j=0; j<LAYOUTSIDE; j++) {
                for (int i=0; i<LAYOUTSIDE; i++) {
                        if (INDEXLAYOUT(i, j, layout)) {
                                fprintf(stderr, "#");
                        } else {
                                fprintf(stderr, ".");
                        }
                }
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
}

__attribute__((unused))
static void print_status(int minute, unsigned layouts[RECURSIONSIZE]) {
        fprintf(stderr, "After %d minutes...\n\n", minute);
        for (size_t i=0; i<RECURSIONSIZE; i++) {
                if (layouts[i]) {
                        fprintf(stderr, "Depth %d\n", (int)i - RECURSIONSIZE/2);
                        print(layouts[i]);
                }
        }
}

static unsigned advance(unsigned layout) {
        unsigned new = 0;
        for (int j=0; j<LAYOUTSIDE; j++) {
                for (int i=0; i<LAYOUTSIDE; i++) {
                        // count adjacent bugs
                        int count =
                                (i+1 < LAYOUTSIDE && INDEXLAYOUT(i+1,j,layout)) +
                                (i-1 >= 0         && INDEXLAYOUT(i-1,j,layout)) +
                                (j+1 < LAYOUTSIDE && INDEXLAYOUT(i,j+1,layout)) +
                                (j-1 >= 0         && INDEXLAYOUT(i,j-1,layout));
                        
                        if (INDEXLAYOUT(i,j,layout)) { // there's a bug, does it die?
                                if (count == 1) { // it lives
                                        new |= GETMASK(i,j);
                                } else { // it dies
                                }
                        } else { // there's not a bug, create one?
                                if (count == 1 || count == 2) { // create one
                                        new |= GETMASK(i,j);
                                } else { // don't
                                }
                                        
                        }
                }
        }
        return new;
}

static int advance2_cnt(unsigned layout, unsigned inner, unsigned outter, int x, int y) {
        int count = 0;

        // Count normal tiles (not past the edge, not the tile at 2,2)
        count += (x+1 < LAYOUTSIDE && !(x+1 == 2 && y == 2) && INDEXLAYOUT(x+1,y,layout));
        count += (x-1 >= 0         && !(x-1 == 2 && y == 2) && INDEXLAYOUT(x-1,y,layout));
        count += (y+1 < LAYOUTSIDE && !(x == 2 && y+1 == 2) && INDEXLAYOUT(x,y+1,layout));
        count += (y-1 >= 0         && !(x == 2 && y-1 == 2) && INDEXLAYOUT(x,y-1,layout));

        //Count tile at the edge
        count += (x == 0            && INDEXLAYOUT(1,2,outter));
        count += (x == LAYOUTSIDE-1 && INDEXLAYOUT(3,2,outter));
        count += (y == 0            && INDEXLAYOUT(2,1,outter));
        count += (y == LAYOUTSIDE-1 && INDEXLAYOUT(2,3,outter));

        // Count inner tiles
        if ((x == 2 && (y == 1 || y == 3)) ||
            (y == 2 && (x == 1 || x == 3))) {
                for (int i=0; i<LAYOUTSIDE; i++) {
                        int ii,jj;
                        
                        if (x == 2) {
                                ii = i;
                        } else if (x == 1) {
                                ii = 0;
                        } else if (x == 3) {
                                ii = LAYOUTSIDE - 1;
                        } else {
                                break;
                        }
                        
                        if (y == 2) {
                                jj = i;
                        } else if (y == 1) {
                                jj = 0;
                        } else if (y == 3) {
                                jj = LAYOUTSIDE - 1;
                        } else {
                                break;
                        }
                        
                        count += INDEXLAYOUT(ii,jj,inner);
                }
        }

        return count;
}

static unsigned advance2_com(unsigned layout, unsigned inner, unsigned outter) {
        unsigned new = 0;

        for (int j=0; j<LAYOUTSIDE; j++) {
                for (int i=0; i<LAYOUTSIDE; i++) {
                        if (i == 2 && j == 2) {
                                continue;
                        }
                        
                        int count = advance2_cnt(layout, inner, outter, i, j);
                        if (INDEXLAYOUT(i,j,layout)) { // there's a bug, does it die?
                                if (count == 1) {      // it lives!
                                        new |= GETMASK(i,j);
                                } else {               // it dies...
                                }
                        } else {                       // there's no bug, get one?
                                if (count == 1 ||
                                    count == 2) {      // get one!
                                        new |= GETMASK(i,j);
                                } else {               // do not...
                                }
                        }
                }
        }

        return new;
}

static void advance2_rec(unsigned src[RECURSIONSIZE], unsigned dst[RECURSIONSIZE],
                         size_t i, bool in, bool out) {
        ASSERT(i > 1 && i+1 < RECURSIONSIZE, "Out of space. %lu", i);
        dst[i] = advance2_com(src[i], src[i+1], src[i-1]);
        if (in && i+2 < RECURSIONSIZE) {
                advance2_rec(src, dst, i+1, true, false);
        }
        if (out && i > 2) {
                advance2_rec(src, dst, i-1, false, true);
        }
}

static void advance2(unsigned layouts[RECURSIONSIZE]) {
        static unsigned tmplayouts[RECURSIONSIZE];
        advance2_rec(layouts, tmplayouts, RECURSIONSIZE/2, true, true);
        memcpy(layouts, tmplayouts, RECURSIONSIZE * sizeof layouts[0]);
}

static unsigned parse_layout(const char *const input) {
        int j = 0;
        unsigned result = 0;
        for (size_t i=0;; i++) {
                char c = input[i];
                if (c == '\0') {
                        break;
                } else if (c == '\n') {
                } else if (c == '#') {
                        result |= 1U<<j;
                        j++;
                } else if (c == '.') {
                        j++;
                } else {
                        FAIL("Unexpected input.");
                }
        }
        ASSERT(j == LAYOUTSIZE, "Unexpected input.");
        return result;
}

static unsigned bitcount(unsigned num) {
        unsigned count = 0;
        for (size_t i=0; i<sizeof(num)*8; i++) {
                if (num & 1U<<i) {
                        count++;
                }
        }
        return count;
}

static unsigned bugcount(unsigned layouts[RECURSIONSIZE]) {
        unsigned count = 0;
        for (size_t i=0; i<RECURSIONSIZE; i++) {
                if (layouts[i]) {
                        count += bitcount(layouts[i]);
                }
        }
        return count;
}

static void solution1(const char *const input, char *const output) {
        unsigned layout = parse_layout(input);
        static bool seen[1<<LAYOUTSIZE];

        do {
                seen[layout] = true;
                layout = advance(layout);
        } while (!seen[layout]);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", layout);
}

static void solution2(const char *const input, char *const output) {
        static unsigned layouts[RECURSIONSIZE];
        layouts[RECURSIONSIZE/2] = parse_layout(input);

        for (int minute=0; minute<200; minute++) {
                advance2(layouts);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", bugcount(layouts));
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
