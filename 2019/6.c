#include <aoclib.h>
#include <stdio.h>

#define NUM_SYMBOLS (('Z'-'A'+1)+('9'-'0'+1))

#define MAX_PLANETS ((NUM_SYMBOLS-1)+\
                     (NUM_SYMBOLS-1)*NUM_SYMBOLS+\
                     (NUM_SYMBOLS-1)*NUM_SYMBOLS*NUM_SYMBOLS)

#define SYMBOLTONUM(c)                                  \
        (c <= '9' ? c - '0' : c - 'A' + 10)

#define NAME2INDEX(name)                                \
        (SYMBOLTONUM(name[0])+                          \
         SYMBOLTONUM(name[1])*NUM_SYMBOLS+              \
         SYMBOLTONUM(name[2])*NUM_SYMBOLS*NUM_SYMBOLS)

static void parse(const char *input, int *planets) {
        while (*input != '\0') {
                int orbitee = NAME2INDEX(input);
                
                input += 3;
                ASSERT(*input == ')', "Expected ) input but got %c %c%c%c[%c]%c%c%c", *input, input[-3],input[-2],input[-1],input[0],input[1],input[2],input[3]);
                
                input++;
                int orbiter = NAME2INDEX(input);
                planets[orbiter] = orbitee;
                
                input += 4;
        }
}

static void calculate_total_orbits(int i, int *planets, int *totals) {
        if (totals[i] > -1) return;
        if (planets[i] == -1) return;

        int orbitee = planets[i];
        if (orbitee != -1) {
                calculate_total_orbits(orbitee, planets, totals);
                ASSERT(totals[orbitee] > -1, "Did not calculate totals for planet %d", orbitee);
                totals[i] = totals[orbitee] + 1;
        } else {
                totals[i] = 0;
        }
}

static void solution1(const char *const input, char *const output) {
        static int planets[MAX_PLANETS];
        static int total_orbits[MAX_PLANETS];
        for (int i=0; i<MAX_PLANETS; i++) {
                planets[i] = -1;
                total_orbits[i] = -1;
        }
        total_orbits[NAME2INDEX("COM")] = 0;
        
        parse(input, planets);
        
        int sum = 0;
        for (int i=0; i<MAX_PLANETS; i++) {
                calculate_total_orbits(i, planets, total_orbits);
                if (total_orbits[i] > -1) {
                        sum += total_orbits[i];
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", sum);
}

static void solution2(const char *const input, char *const output) {
        static int planets[MAX_PLANETS];
        static int paths[MAX_PLANETS];
        for (int i=0; i<MAX_PLANETS; i++) {
                planets[i] = -1;
                paths[i] = -1;
        }

        parse(input, planets);

        // Navigate the whole path from where we are to the COM all
        // the while marking how far away we are from our origin
        int count = 0;
        int current = planets[NAME2INDEX("YOU")];
        ASSERT(current != -1, "YOU is not orbiting any planets?");
        do {
                paths[current] = count++;
                current = planets[current];
                ASSERT(current != -1, "Chain 1 broken?");
        } while (current != NAME2INDEX("COM"));

        // Navigate the path from where Santa is until we reach a
        // planet marked by the previous path. At this point we know
        // the length of the full path.
        count = 0;
        current = planets[NAME2INDEX("SAN")];
        ASSERT(current != -1, "SAN is not orbiting any planets?");
        while (paths[current] == -1) {
                count++;
                current = planets[current];
                ASSERT(current != -1, "Chain 2 broken?");
        }

        int result = count + paths[current];
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
