#include <aoclib.h>
#include <stdio.h>

#define MAP_HEIGHT 323
#define MAP_WIDTH 31

static bool map[MAP_HEIGHT * MAP_WIDTH];

static void parse_map(const char *const input) {
        for (int j=0; j<MAP_HEIGHT; j++) {
                for (int i=0; i<MAP_WIDTH; i++) {
                        char c = input[j*(MAP_WIDTH+1)+i];
                        if (c == '#') {
                                map[j*MAP_WIDTH+i] = true;
                        } else if (c == '.') {
                                map[j*MAP_WIDTH+i] = false;
                        } else {
                                FAIL("unexpected character: %c (%d)", c, c);
                        }
                }
                ASSERT(input[j*(MAP_WIDTH+1)+MAP_WIDTH] == '\n', "unexpected input");
        }
        ASSERT(input[MAP_HEIGHT*(MAP_WIDTH+1)+0] == '\0', "unexpected input");
}

static int tree_count(int advi, int advj) {
        int i = 0;
        int j = 0;
        int count = 0;
        
        for(;;) {
                i += advi;
                j += advj;
                i %= MAP_WIDTH;

                if (j >= MAP_HEIGHT) {
                        break;
                }

                if (map[j*MAP_WIDTH+i]) {
                        count++;
                }
        }

        return count;
}

static void solution1(const char *const input, char *const output) {
        parse_map(input);
        int count = tree_count(3, 1);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static void solution2(const char *const input, char *const output) {
        parse_map(input);

        struct {int advi; int advj;} coords[] = {
                {1, 1},
                {3, 1},
                {5, 1},
                {7, 1},
                {1, 2}
        };
        unsigned ncoords = sizeof(coords) / sizeof(*coords);

        long result = 1;
        for (unsigned i=0; i<ncoords; i++) {
                result *= (long)tree_count(coords[i].advi, coords[i].advj);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
