#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define DIMENSION 200
#define LOWERLIMIT 50
#define IDX(x,y) ((y)*DIMENSION+(x))

enum cardinal {
        NORTH,
        SOUTH,
        WEST,
        EAST,
};

static void parse_input(const char *input, bool **points) {
        *points = malloc(sizeof(**points)*DIMENSION*DIMENSION);
        memset(*points, 0, sizeof(**points)*DIMENSION*DIMENSION);

        int i=0;
        int j=0;
        while (*input != '\0') {
                char c = *input;
                if (c == '\n') {
                        input++;
                        i=0;
                        j++;
                        while (*input == '\n') {
                                input++;   
                        }
                } else {
                        if (c == '#') {
                                (*points)[IDX(i+50,j+50)] = true;
                        } else {
                                ASSERT(c == '.', "parse error");
                        }
                        i++;
                        input++;
                }
        }
}

static void move_coord(enum cardinal dir, int *x, int *y) {
        switch (dir) {
        case NORTH:
                *y -= 1;
                ASSERT(*y >= 0, "lower limit too low");
                break;
        case SOUTH:
                *y += 1;
                ASSERT(*y < DIMENSION, "dimension too low");
                break;
        case WEST:
                *x -= 1;
                ASSERT(*x >= 0, "lower limit too low");
                break;
        case EAST:
                *x += 1;
                ASSERT(*x < DIMENSION, "dimension too low");
                break;
        default:
                FAIL("invalid dir");
        }
}

static bool spread(bool *points) {
        static enum cardinal start = NORTH;
        
        enum cardinal *elf_direction = malloc(sizeof(*elf_direction)*DIMENSION*DIMENSION);

        int *elf_next_position = malloc(sizeof(*elf_next_position)*DIMENSION*DIMENSION);
        memset(elf_next_position, 0, sizeof(*elf_next_position)*DIMENSION*DIMENSION);

        for (int j=0; j<DIMENSION; j++) {
                for (int i=0; i<DIMENSION; i++) {
                        if (!points[IDX(i,j)]) {
                                continue;
                        }
                        elf_direction[IDX(i,j)] = -1;
                
                        bool alone = true;
                        for (int jj=-1; jj<=1; jj++) {
                                for (int ii=-1; ii<=1; ii++) {
                                        if (jj == 0 && ii == 0) {
                                                continue;
                                        }
                                        alone &= !points[IDX(i+ii, j+jj)];
                                        DBG("add %d,%d to %d,%d, we get %d", ii, jj, i, j, points[IDX(i+ii, j+jj)]);
                                        if (!alone) {
                                                break;
                                        }
                                }
                                if (!alone) {
                                        break;

                                }
                        }
                        if (alone) {
                                DBG("elf at %d,%d is alone and won't move", i, j);
                                continue;
                        }
                
                        enum cardinal proposal_dir;
                        bool can_move = false;
                        for (int k=0; k<4; k++) {
                                enum cardinal dir = (start + k) % 4;
                                proposal_dir = dir;
                                
                                int x = i;
                                int y = j;

                                // go north/south/west/east
                                move_coord(dir, &x, &y);
                                if (points[IDX(x,y)]) {
                                        continue;
                                }
                                
                                // now go west/west/north/north
                                if (dir / 2 == 0) {
                                        dir = 2;
                                } else {
                                        dir = 0;
                                }
                                move_coord(dir, &x, &y);
                                if (points[IDX(x,y)]) {
                                        continue;
                                }
                                
                                // now go east/east/south/south twice
                                dir++;
                                move_coord(dir, &x, &y);
                                move_coord(dir, &x, &y);
                                if (points[IDX(x,y)]) {
                                        continue;
                                }

                                can_move = true;
                                break;
                        }
                        if (!can_move) {
                                DBG("elf at %d,%d has no valid move", i, j);
                                continue;
                        }

                        int x = i;
                        int y = j;
                        move_coord(proposal_dir, &x, &y);
                        elf_direction[IDX(i,j)] = proposal_dir;
                        elf_next_position[IDX(x, y)]++;
                        DBG("elf at %d,%d has proposed to move %s (to %d,%d)", i, j, ((const char*[]){"north","south","west","east"})[proposal_dir], x, y);
                }
        }

        bool *new_points = malloc(sizeof(*new_points)*DIMENSION*DIMENSION);
        memset(new_points, 0, sizeof(*new_points)*DIMENSION*DIMENSION);
        
        bool done = true;
        for (int j=0; j<DIMENSION; j++) {
                for (int i=0; i<DIMENSION; i++) {
                        if (!points[IDX(i,j)]) {
                                continue;
                        }
                        
                        int next_dir_int = elf_direction[IDX(i,j)];
                        if (next_dir_int == -1) {
                                DBG("elf at %d,%d does not move", i, j);
                                new_points[IDX(i,j)] = true;
                                continue;
                        }
                        enum cardinal next_dir = next_dir_int;

                        int x = i;
                        int y = j;
                        move_coord(next_dir, &x, &y);
                        if (elf_next_position[IDX(x, y)] > 1) {
                                DBG("elf at %d,%d wanted to move %s to %d,%d but cannot as other elves also want to move there", i, j, ((const char*[]){"north","south","west","east"})[next_dir], x, y);
                                new_points[IDX(i,j)] = true;
                                continue;
                        }
                        DBG("elf at %d,%d moves %s to %d,%d", i, j, ((const char*[]){"north","south","west","east"})[next_dir], x, y);

                        new_points[IDX(x, y)] = true;
                        done = false;
                }
        }
        memcpy(points, new_points, sizeof(*new_points)*DIMENSION*DIMENSION);
        free(new_points);
        
        start = (start + 1) % 4;
        free(elf_direction);
        free(elf_next_position);
        return done;
}

static void dbg_prnt_points(const bool *points, bool cool) {
#ifdef DEBUG
        for (int j=0; j<12; j++) {
                for (int i=0; i<14; i++) {
                        char c;
                        if (points[IDX(i+LOWERLIMIT,j+LOWERLIMIT)]) {
                                c = '#'; 
                        } else {
                                c = '.';
                        }
                        fputc(c, stderr);
                }
                fputc('\n', stderr);
        }
        fputc('\n', stderr);

        if (cool) {
                fputs("\033[F", stderr);
                for (int j=0; j<DIMENSION; j++) {
                        fputs("\033[A", stderr);
                }
        }
#else
        (void)points;
        (void)cool;
#endif
}

static void solution1(const char *const input, char *const output) {
        bool *points;
        parse_input(input, &points);

        dbg_prnt_points(points, false);

        for (int round=0; round<10; round++) {
                spread(points);
                DBG("End of round %d", round+1);
                dbg_prnt_points(points, false);
        }

        int maxx = INT_MIN;
        int minx = INT_MAX;
        int maxy = INT_MIN;
        int miny = INT_MAX;
        int len = 0;
        for (int i=0; i<DIMENSION; i++) {
                for (int j=0; j<DIMENSION; j++) {
                        if (points[IDX(i,j)]) {
                                if (i > maxx) {
                                        maxx = i;
                                }
                                if (i < minx) {
                                        minx = i;
                                }
                                if (j > maxy) {
                                        maxy = j;
                                }
                                if (j < miny) {
                                        miny = j;
                                }
                                len++;
                        }
                }
        }
        int size = (maxx - minx + 1) * (maxy - miny + 1);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", size - len);
        free(points);
}

static void solution2(const char *const input, char *const output) {
        bool *points;
        parse_input(input, &points);

        int round;
        for (round=0;; round++) {
                bool stop = spread(points);
                //dbg_prnt_points(points, len, -1, -1, -1, -1, true);
                if (stop) {
                        break;
                }
        }

        //dbg_prnt_points(points, len, -1, -1, -1, -1, false);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", round + 1);
        free(points);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
