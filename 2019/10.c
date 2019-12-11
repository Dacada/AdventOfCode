#define _GNU_SOURCE // qsort_r

#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define INDEX(rws,cls,r,c) ((c)+(r)*(cls))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define ABS(x) ((x) < 0 ? -(x) : (x))

static char *parse_map(const char *const input, int *const rows, int *const columns) {
        int size = 1014;
        char *map = malloc(size * sizeof(*map));
        if (map == NULL) {
                perror("malloc");
                return NULL;
        }

        *rows = 0;
        int j = 0;
        for (int i=0;; i++) {
                char c = input[i];
                switch (c) {
                case '\0':
                        map = realloc(map, j);
                        if (map == NULL) {
                                perror("realloc");
                                return NULL;
                        }
                        *columns = j / *rows;
                        return map;

                case '\n':
                        *rows += 1;
                        break;
                default:
                        while (j > size) {
                                size *= 2;
                                map = realloc(map, size);
                                if (map == NULL) {
                                        perror("realloc");
                                        return NULL;
                                }
                        }
                        map[j++] = input[i];
                }
        }
}

static void make_coprime(int a, int b,
                         int *const resA, int *const resB) {
        for (int d = MAX(a,b); d > 1; d--) {
                if (a % d == 0 && b % d == 0) {
                        a /= d;
                        b /= d;
                }
                if (a == 1 || b == 1) {
                        break;
                }
        }
        *resA = a;
        *resB = b;
}

static void draw_line(int x1,int y1, int x2,int y2, int *xr,int *yr) {
        int dx = ABS(x1 - x2);
        int dy = ABS(y1 - y2);
        make_coprime(dx, dy, xr, yr);
}

static bool visible(const int goalx, const int goaly,
                    const int origx, const int origy,
                    const char *const map, const int rows, const int cols) {
        // Is goal visible from orig?

        // Draw a line from orig to goal. This function gives us a
        // length x,y of each segment of the line. While there are
        // many possible lines, we care only about the "jumps" where
        // segments meet.
        //
        // . . . . . . . . .
        // . #$$$$$$ . . . .
        // . % . . $ . . . .
        // . %%%%%%X$$$$$$ .
        // . . . . % . . $ .
        // . . . . %%%%%%# .
        // . . . . . . . . .
        // distance from (1,1) to (7,5) => 6,4 => 3,2
        //
        // For this example, we'd get 3,1 and these are two of the
        // possible lines we could trace, but both meet at X after 3
        // left and 2 down.

        //DBG("Trying to check if %d,%d is visible from %d,%d", goalx,goaly, origx,origy);

        int segx, segy;
        draw_line(origx, origy, goalx, goaly, &segx, &segy);
        
        // Fix signs: if orig is greater than goal then seg should be
        // negative.
        if (origx > goalx) {
                segx = -segx;
        }
        if (origy > goaly) {
                segy = -segy;
        }

        //DBG("Minimum segment is %d,%d", segx, segy);

        // Now we jump by segx,segy through the map starting and
        // origx,origy until we reach goalx,goaly. If at any step we
        // find an asteroid, we stop and return false. Otherwise we
        // can return true.
        int i = origx+segx;
        int j = origy+segy;
        while (i >= 0 && i < cols && j >=0 && j < rows) {
                char c = map[INDEX(rows,cols,j,i)];
                
                //DBG("%c at %d,%d", c, i,j);
                if (c == '#') {
                        if (j == goaly && i == goalx) {
                                //DBG("It's visible.");
                                return true;
                        } else {
                                //DBG("It's not visible");
                                return false;
                        }
                }

                i += segx;
                j += segy;
        }

        FAIL("Did not find the goal by jumping around!");
}

static int get_best_position(const char *const map,
                             const int rows, const int cols,
                             int *const bestx, int *const besty) {
        int best_visible = 0;
        for (int j1=0; j1<rows; j1++) {
                for (int i1=0; i1<cols; i1++) {
                        if (map[INDEX(rows, cols, j1, i1)] == '#') {
                                // i1,j1 now points to an asteroid
                                int nvisible = 0;
                                
                                for (int j2=0; j2<rows; j2++) {
                                        for (int i2=0; i2<cols; i2++) {
                                                // ensure they're different asteroids
                                                if (j1 != j2 || i1 != i2) {
                                                        if (map[INDEX(rows, cols, j2, i2)] == '#') {
                                                                // i2,j2 now points to an asteroid
                                                                if (visible(i2,j2, i1,j1, map,rows,cols)) {
                                                                        nvisible++;
                                                                }
                                                        }
                                                }
                                        }
                                }

                                if (best_visible < nvisible) {
                                        best_visible = nvisible;
                                        *bestx = i1;
                                        *besty = j1;
                                }
                        }
                }
        }

        return best_visible;
}

static int cmp_int(const int a, const int b) {
        if (a < b) {
                return -1;
        } else if (a > b) {
                return 1;
        } else {
                return 0;
        }
}

static int sign_int(const int x) {
        if (x < 0) {
                return -1;
        } else if (x > 0) {
                return 1;
        } else {
                return 0;
        }
}

static int vec_cross(const int ax, const int ay, const int bx, const int by) {
        return ax * by - ay * bx;
}

static int cmp_angle(const void *vptr_v1, const void *vptr_v2, void *vptr_org) {
        int v1_x, v1_y, v2_x, v2_y;
        {
                const int *ptr_v1 = vptr_v1;
                const int *ptr_v2 = vptr_v2;
                const int *ptr_org = vptr_org;

                int v1 = *ptr_v1;
                int v2 = *ptr_v2;
                int org = *ptr_org;

                int v1x = v1/100;
                int v1y = v1%100;
        
                int v2x = v2/100;
                int v2y = v2%100;

                int orgx = org/100;
                int orgy = org%100;

                v1_x = v1x - orgx;
                v1_y = v1y - orgy;
                
                v2_x = v2x - orgx;
                v2_y = v2y - orgy;
        }

        int dv1 = v1_x < 0;
        int dv2 = v2_x < 0;

        if (dv1 != dv2) {
                return cmp_int(dv1, dv2);
        } else if (v1_x == 0 && v2_x == 0) {
                return cmp_int(sign_int(v1_y), sign_int(v2_y));
        } else {
                return sign_int(vec_cross(v2_x, v2_y, v1_x, v1_y));
        }
}

static void solution1(const char *const input, char *const output) {
        int rows, cols;
        char *map = parse_map(input, &rows, &cols);
        if (map == NULL) {
                snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
                return;
        }
        
        int nvisible, x, y;
        nvisible = get_best_position(map, rows, cols, &x, &y);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", nvisible);

        free(map);
}

static void solution2(const char *const input, char *const output) {
        int rows, cols;
        char *map = parse_map(input, &rows, &cols);
        if (map == NULL) {
                snprintf(output, OUTPUT_BUFFER_SIZE, "MEMORY ERROR");
                return;
        }
        
        int x, y;
        int nvisible = get_best_position(map, rows, cols, &x, &y);
        
        int *visible_arr = malloc(sizeof(*visible_arr) * nvisible);
        int k = 0;
        for (int j=0; j<rows; j++) {
                for (int i=0; i<cols; i++) {
                        if (map[INDEX(rows, cols, j, i)] == '#' &&
                            !(x == i && y == j) &&
                            visible(x,y,i,j,map,rows,cols)) {
                                visible_arr[k++] = i*100+j;
                        }
                }
        }

        int origin = x*100+y;
        qsort_r(visible_arr, nvisible, sizeof(*visible_arr), cmp_angle, &origin);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", visible_arr[200-1]);
        free(map);
        free(visible_arr);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
