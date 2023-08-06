#include <aoclib.h>
#include <stdio.h>

enum position {
        floor,
        empty_seat,
        full_seat
};

size_t lenx, leny;
enum position *grid;
enum position *other_grid;
static void gridswap() {
        enum position *tmp = grid;
        grid = other_grid;
        other_grid = tmp;
}

static void parse(const char *input) {
        enum position *grid1 = NULL;
        size_t capacity;
        
        for(lenx=0; input[lenx]!='\n'; lenx++);
        leny = 0;
        capacity = 16;
        grid1 = malloc(sizeof(*grid1)*lenx*capacity);

        while (*input!='\0') {
                if (leny >= capacity) {
                        capacity *= 2;
                        grid1 = realloc(grid1, sizeof(*grid1)*lenx*capacity);
                }
                
                for (size_t i=0; i<lenx; i++) {
                        enum position seat;
                        switch (*input) {
                        case '.':
                                seat = floor;
                                break;
                        case 'L':
                                seat = empty_seat;
                                break;
                        default:
                                FAIL("parse error");
                        }
                        grid1[lenx*leny+i] = seat;
                        input++;
                }
                
                ASSERT(*input=='\n', "parse error");
                input++;
                leny++;
        }

        enum position *grid2 = malloc(sizeof(*grid2)*lenx*leny);

        grid = grid1;
        other_grid = grid2;
}

static unsigned count_occupied_neighbors_1(size_t i, size_t j) {
        unsigned count = 0;
        for (int jj=-1; jj<=1; jj++) {
                if (jj < 0 && j == 0) {
                        continue;
                }
                if (j+jj >= leny) {
                        continue;
                }
                for (int ii=-1; ii<=1; ii++) {
                        if (ii < 0 && i == 0) {
                                continue;
                        }
                        if (i+ii >= lenx) {
                                continue;
                        }
                        if (ii == 0 && jj == 0) {
                                continue;
                        }
                        enum position seat = grid[(j+jj)*lenx+(i+ii)];
                        if (seat == full_seat) {
                                count++;
                        }
                }
        }
        return count;
}

__attribute__((pure))
static unsigned count_occupied_neighbors_2(size_t ui, size_t uj) {
        int i = ui;
        int j = uj;
        
        unsigned count = 0;
        for (int dj=-1; dj<=1; dj++) {
                for (int di=-1; di<=1; di++) {
                        if (di == 0 && dj == 0) {
                                continue;
                        }

                        int ii = di;
                        int jj = dj;
                        while (i+ii >= 0 && i+ii < (int)lenx &&
                               j+jj >= 0 && j+jj < (int)leny) {
                                enum position seat = grid[(j+jj)*lenx+(i+ii)];
                                if (seat == full_seat) {
                                        count++;
                                        break;
                                } else if (seat == empty_seat) {
                                        break;
                                }
                                ii += di;
                                jj += dj;
                        }
                }
        }
        return count;
}

static bool step(unsigned(*do_count)(size_t,size_t), unsigned neighbor_limit) {
        bool changed = false;
        for (size_t j=0; j<leny; j++) {
                for (size_t i=0; i<lenx; i++) {
                        unsigned neighbors = do_count(i, j);
                        switch (grid[j*lenx+i]) {
                        case floor:
                                other_grid[j*lenx+i] = grid[j*lenx+i];
                                break;
                        case empty_seat:
                                if (neighbors == 0) {
                                        changed = true;
                                        other_grid[j*lenx+i] = full_seat;
                                } else {
                                        other_grid[j*lenx+i] = grid[j*lenx+i];
                                }
                                break;
                        case full_seat:
                                if (neighbors >= neighbor_limit) {
                                        changed = true;
                                        other_grid[j*lenx+i] = empty_seat;
                                } else {
                                        other_grid[j*lenx+i] = grid[j*lenx+i];
                                }
                        }
                }
        }
        gridswap();
        return changed;
}

__attribute__((pure))
static unsigned count_occupied_seats(void) {
        unsigned count = 0;
        for (size_t j=0; j<leny; j++) {
                for (size_t i=0; i<lenx; i++) {
                        enum position seat = grid[j*lenx+i];
                        if (seat == full_seat) {
                                count++;
                        }
                }
        }
        return count;
}

static void solution1(const char *const input, char *const output) {
        parse(input);
        while (step(count_occupied_neighbors_1, 4));
        unsigned count = count_occupied_seats();
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
        free(grid);
        free(other_grid);
}

static void solution2(const char *const input, char *const output) {
        parse(input);
        while (step(count_occupied_neighbors_2, 5));
        unsigned count = count_occupied_seats();
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
        free(grid);
        free(other_grid);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
