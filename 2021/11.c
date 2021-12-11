#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static void parse_input(const char *input, unsigned octopuses[10][10]) {
        for (unsigned j=0; j<10; j++) {
                for (unsigned i=0; i<10; i++) {
                        octopuses[j][i] = *input - '0';
                        input++;
                }
                ASSERT(*input == '\n', "parse error");
                input++;
        }
        while (*input == '\n') {
                input++;
        }
        ASSERT(*input == '\0', "parse error");
}

static unsigned energize(unsigned octopuses[10][10], bool flashes[10][10], unsigned x, unsigned y) {
        if (flashes[y][x]) {
                return 0;
        }
        
        unsigned energy = ++octopuses[y][x];
        if (energy <= 9) {
                return 0;
        }

        flashes[y][x] = true;
        octopuses[y][x] = 0;
                
        unsigned count = 0;
        for (int j=-1; j<=1; j++) {
                for (int i=-1; i<=1; i++) {
                        int newx = (int)x+i;
                        int newy = (int)y+j;
                        if (newx >= 0 && newx < 10 && newy >= 0 && newy < 10) {
                                count += energize(octopuses, flashes, newx, newy);
                        }
                }
        }
        return count + 1;
}

static unsigned step(unsigned octopuses[10][10]) {
        static bool flashes[10][10];
        
        unsigned count = 0;
        for (unsigned j=0; j<10; j++) {
                for (unsigned i=0; i<10; i++) {
                        count += energize(octopuses, flashes, i, j);
                }
        }
        memset(flashes, 0, 10*10*sizeof(bool));
        
        return count;
}

static void solution1(const char *const input, char *const output) {
        static unsigned octopuses[10][10];
        parse_input(input, octopuses);

        unsigned count = 0;
        for (unsigned i=0; i<100; i++) {
                count += step(octopuses);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
}

static void solution2(const char *const input, char *const output) {
        static unsigned octopuses[10][10];
        parse_input(input, octopuses);

        unsigned i;
        for (i=1;; i++) {
                unsigned count = step(octopuses);
                if (count == 100) {
                        break;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", i);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
