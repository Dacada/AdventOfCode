#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define ABS(x) ((x)>=0?(x):-(x))

struct coord {
        int x, y, z, w;
};

struct grid {
        bool *grids;
        struct coord dims;
};

static size_t grid_size(const struct grid *const grid) {
        return grid->dims.x * grid->dims.y * grid->dims.z * grid->dims.w * 16 * sizeof(*grid->grids);
}

static void init_grid(struct grid *const grid, struct coord dims) {
        grid->dims = dims;
        size_t size = grid_size(grid);
        grid->grids = malloc(size);
        memset(grid->grids, 0, size);
}

static int grid_idx(struct coord coords, const struct coord dims) {
        int quadrant = 0;
        
        if (coords.x < 0) {
                quadrant |= 1<<0;
                coords.x = -coords.x - 1;
        }
        if (coords.y < 0) {
                quadrant |= 1<<1;
                coords.y = -coords.y - 1;
        }
        if (coords.z < 0) {
                quadrant |= 1<<2;
                coords.z = -coords.z - 1;
        }
        if (coords.w < 0) {
                quadrant |= 1<<3;
                coords.w = -coords.w - 1;
        }
        
        return  coords.x +
                coords.y * dims.x +
                coords.z * dims.y * dims.x +
                coords.w * dims.z * dims.y * dims.x +
                quadrant * dims.w * dims.z * dims.y * dims.x;
}

static struct coord grid_unidx(int idx, const struct coord dims) {
        int quadrant = idx / (dims.w * dims.z * dims.y * dims.x);
        
        int allcoords = idx % (dims.w * dims.z * dims.y * dims.x);
        
        struct coord coords;
        coords.w = allcoords / (dims.z * dims.y * dims.x);
        allcoords = allcoords % (dims.z * dims.y * dims.x);
        coords.z = allcoords / (dims.y * dims.x);
        allcoords = allcoords % (dims.y * dims.x);
        coords.y = allcoords / dims.x;
        coords.x = allcoords % dims.x;

        if (quadrant & (1<<0)) {
                coords.x = -(coords.x + 1);
        }
        if (quadrant & (1<<1)) {
                coords.y = -(coords.y + 1);
        }
        if (quadrant & (1<<2)) {
                coords.z = -(coords.z + 1);
        }
        if (quadrant & (1<<3)) {
                coords.w = -(coords.w + 1);
        }
        
        return coords;
}

static void grow_grid(const struct grid *const from, struct grid *const to, int growth) {
        to->dims = from->dims;
        to->dims.x += growth;
        to->dims.y += growth;
        to->dims.z += growth;
        to->dims.w += growth;
        
        size_t size = grid_size(to);
        to->grids = malloc(size);
        memset(to->grids, 0, size);
}

static void free_grid(struct grid *const grid) {
        free(grid->grids);
}

static bool grid_get(const struct grid *const grid, const struct coord coords) {
        if (ABS(coords.x) >= grid->dims.x || ABS(coords.y) >= grid->dims.y ||
            ABS(coords.z) >= grid->dims.z || ABS(coords.w) >= grid->dims.w) {
                return false;
        }
        
        size_t idx = grid_idx(coords, grid->dims);
        return grid->grids[idx];
}

static void grid_set(struct grid *const grid, struct coord coords, bool value) {
        if (ABS(coords.x) >= grid->dims.x || ABS(coords.y) >= grid->dims.y ||
            ABS(coords.z) >= grid->dims.z || ABS(coords.w) >= grid->dims.w) {
                ASSERT(!value, "bad grid set: (%d,%d,%d,%d)", coords.x, coords.y, coords.z, coords.w);
        }
        
        size_t idx = grid_idx(coords, grid->dims);
        grid->grids[idx] = value;
}

static void parse(const char *input, struct grid *const grid) {
        int i;
        for (i=0;input[i]!='\n';i++);
        
        struct coord dims = { .x=i-1, .y=i-1, .z=1, .w=1 };
        init_grid(grid, dims);
        
        struct coord coords = { .x=-i/2-i%2, .y=-i/2, .z=0, .w=0 };
        
        for (; *input!='\0'; input++) {
                char c = *input;
                switch (c) {
                case '#':
                        coords.x++;
                        grid_set(grid, coords, true);
                        break;
                case '.':
                        coords.x++;
                        grid_set(grid, coords, false);
                        break;
                case '\n':
                        coords.x = -i/2-i%2;
                        coords.y++;
                        break;
                default:
                        FAIL("parse error");
                }
        }
}

static int count_active_neighbors(const struct grid *const grid, struct coord coords) {
        int count = 0;
        for (int l=-1; l<=1; l++) {
                for (int k=-1; k<=1; k++) {
                        for (int j=-1; j<=1; j++) {
                                for (int i=-1; i<=1; i++) {
                                        if (i == 0 && j == 0 && k == 0 && l==0) {
                                                continue;
                                        }
                                        struct coord neighbor = {
                                                .x=coords.x+i,
                                                .y=coords.y+j,
                                                .z=coords.z+k,
                                                .w=coords.w+l,
                                        };
                                        if (grid_get(grid, neighbor)) {
                                                count++;
                                                if (count >= 4) { // save some time maybe
                                                        return count;
                                                }
                                        }
                                }
                        }
                }
        }
        return count;
}

static void advance(struct grid *const grid1, struct grid *const grid2, int dimensions) {
        grow_grid(grid1, grid2, 1);
        for (int idx=0; idx < grid2->dims.x * grid2->dims.y * grid2->dims.z * grid2->dims.w * 16; idx++) {
                struct coord coords = grid_unidx(idx, grid2->dims);
                if (dimensions == 3 && coords.w != 0) {
                        continue;
                }
                                        
                int actives = count_active_neighbors(grid1, coords);
                if (grid_get(grid1, coords)) {
                        if (actives == 2 || actives == 3) {
                                grid_set(grid2, coords, true);
                        } else {
                                grid_set(grid2, coords, false);
                        }
                } else {
                        if (actives == 3) {
                                grid_set(grid2, coords, true);
                        } else {
                                grid_set(grid2, coords, false);
                        }
                }
        }
        free_grid(grid1);
}

static int count_all_active(const struct grid *const grid) {
        int count = 0;
        for (int idx=0; idx < grid->dims.x * grid->dims.y * grid->dims.z * grid->dims.w * 16; idx++) {
                struct coord coords = grid_unidx(idx, grid->dims);
                if (grid_get(grid, coords)) {
                        count++;
                }
        }
        return count;
}

static void print_grid(const struct grid *const grid, int cycle, int dimensions) {
        #ifdef DEBUG
        if (cycle == 0) {
                fprintf(stderr, "Before any cycles:\n\n");
        } else {
                fprintf(stderr, "After %d cycles:\n\n", cycle);
        }
        
        for (int w=-grid->dims.w+1; w<grid->dims.w; w++) {
                if (dimensions == 3 && w != 0) {
                        continue;
                }
                for (int z=-grid->dims.z+1; z<grid->dims.z; z++) {
                        if (dimensions == 3) {
                                fprintf(stderr, "z=%d\n", z);
                        } else {
                                fprintf(stderr, "z=%d,w=%d\n", z, w);
                        }
                        for (int y=-grid->dims.y+1; y<grid->dims.y; y++) {
                                for (int x=-grid->dims.x+1; x<grid->dims.x; x++) {
                                        struct coord coords = {.x=x, .y=y, .z=z, .w=w};
                                        if (grid_get(grid, coords)) {
                                                fprintf(stderr, "#");
                                        } else {
                                                fprintf(stderr, ".");
                                        }
                                }
                                fprintf(stderr, "\n");
                        }
                        fprintf(stderr, "\n");
                }
                fprintf(stderr, "\n");
        }
        #else
        (void)grid;
        (void)cycle;
        (void)dimensions;
        #endif
}

static void solution(const char *const input, char *const output, int dimensions) {
        struct grid grid1, grid2;
        struct grid *g1=&grid1, *g2=&grid2;
        
        parse(input, g1);

        print_grid(g1, 0, dimensions);
        for (int i=0; i<6; i++) {
                advance(g1, g2, dimensions);
                print_grid(g2, i+1, dimensions);
                
                struct grid *tmp = g1;
                g1 = g2;
                g2 = tmp;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count_all_active(g1));
        free_grid(g1);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 3);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 4);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
