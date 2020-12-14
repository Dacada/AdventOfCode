#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define MAXBUSES 256

static unsigned parse_uint(const char **input) {
        unsigned n = 0;
        char c;
        while (isdigit(c = **input)) {
                n = n * 10 + c - '0';
                *input += 1;
        }
        return n;
}

static bool parse_uint_maybe(const char **input, unsigned *num) {
        if (!isdigit(**input)) {
                ASSERT(**input == 'x', "parse error");
                *input += 1;
                return false;
        }
        *num = parse_uint(input);
        return true;
}

static unsigned me;
static unsigned buses[MAXBUSES];
static unsigned nbuses;
static void parse(const char *input) {
        me = parse_uint(&input);
        ASSERT(*input == '\n', "parse error");
        input++;

        unsigned n;
        nbuses = 0;
        while (*input != '\0') {
                if (parse_uint_maybe(&input, &n)) {
                        buses[nbuses++] = n;
                } else {
                        buses[nbuses++] = 0;
                }
                ASSERT(*input == ',' || *input == '\n', "parse error");
                input++;
        }
}

static void solution1(const char *const input, char *const output) {
        parse(input);

        unsigned best_time = -1;
        unsigned best_bus = 0;
        for (unsigned i=0; i<nbuses; i++) {
                unsigned bus = buses[i];
                if (bus == 0) {
                        continue;
                }
                
                unsigned time = bus - me % bus;
                if (time < best_time) {
                        best_time = time;
                        best_bus = bus;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", best_time * best_bus);
}

// Adepted from https://rosettacode.org/wiki/Modular_inverse#C
static unsigned long modular_multiplicative_inverse(long a, long b) {
        long b0 = b, t, q;
	long x0 = 0, x1 = 1;
	if (b == 1) return 1;
	while (a > 1) {
		q = a / b;
		t = b, b = a % b, a = t;
		t = x0, x0 = x1 - q * x0, x1 = t;
	}
	if (x1 < 0) x1 += b0;
	return x1;
}

static unsigned long sub_mod(unsigned long a, unsigned long b, unsigned long m) {
        long r = (long)(a % m) - (long)(b % m);
        return (r + (long)m) % m;
}

/*
 * (from example) find x such that x mod 7 \equiv 0, x mod 13 \equiv 12, ... x
 * \mod bus[i] = bus[i] - i
 *
 * Chinese reminder theorem!
 * Algorithm from: https://brilliant.org/wiki/chinese-remainder-theorem/
 */
static void solution2(const char *const input, char *const output) {
        parse(input);

        // 1. N = \prod_{i=1}^k n_i
        unsigned long N = 1;
        for (unsigned i=0; i<nbuses; i++) {
                unsigned bus = buses[i];
                if (bus > 0) {
                        N *= bus;
                }
        }
        DBG("N = %lu", N);

        // 2. y_i = N / n_i
        unsigned long y[MAXBUSES];
        for (unsigned i=0; i<nbuses; i++) {
                unsigned bus = buses[i];
                if (bus > 0) {
                        y[i] = N / bus;
                        DBG("y_%u = %lu", i, y[i]);
                }
        }

        // 3. z_i \equiv y_i^{-1} mod n_i
        unsigned long z[MAXBUSES];
        for (unsigned i=0; i<nbuses; i++) {
                unsigned bus = buses[i];
                if (bus > 0) {
                        z[i] = modular_multiplicative_inverse(y[i], bus);
                        DBG("z_%u = %lu", i, z[i]);
                }
        }

        // 4. x = \sum_{i=1}^k a_i y_i z_i
        unsigned long time = 0;
        for (unsigned i=0; i<nbuses; i++) {
                unsigned bus = buses[i];
                if (bus > 0) {
                        unsigned long a = sub_mod(bus, i, bus);
                        DBG("a_%u = %lu", i, a);
                        time += a * y[i] * z[i];
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", time % N);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
