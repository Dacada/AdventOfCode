#include <aoclib.h>
#include <stdio.h>

//#define BRAINLET_MODE
static __int128 deck_size;

#ifdef BRAINLET_MODE

static int *deck;
static int *tmpdeck;
static int *tmpdeck2;

static void factory_order(void) {
        for (__int128 i=0; i<deck_size; i++) {
                deck[i] = i;
        }
}
static void deal_into_new_stack(__int128 _) {
        (void)_;
        for (__int128 i=0; i<deck_size/2; i++) {
                __int128 j = deck_size - i - 1;
                __int128 tmp = deck[i];
                deck[i] = deck[j];
                deck[j] = tmp;
        }
}
static void cut_cards(__int128 n) {
        if (n > 0) {
                for (__int128 i=0; i<n; i++) {
                        tmpdeck[i] = deck[i];
                }
                for (__int128 i=n; i<deck_size; i++) {
                        tmpdeck2[i-n] = deck[i];
                }
                for (__int128 i=0; i<n; i++) {
                        tmpdeck2[deck_size-n+i] = tmpdeck[i];
                }
                for (__int128 i=0; i<deck_size; i++){
                        deck[i] = tmpdeck2[i];
                }
        } else if (n < 0) {
                n = -n;
                for (__int128 i=deck_size-n; i<deck_size; i++) {
                        tmpdeck[i] = deck[i];
                }
                for (__int128 i=0; i<deck_size-n; i++) {
                        tmpdeck2[i+n] = deck[i];
                }
                for (__int128 i=deck_size-n; i<deck_size; i++) {
                        tmpdeck2[i-deck_size+n] = tmpdeck[i];
                }
                for (__int128 i=0; i<deck_size; i++){
                        deck[i] = tmpdeck2[i];
                }
        }
}
static void deal_with_increment(__int128 n) {
        __int128 pos = 0;
        for (__int128 i=0; i<deck_size; i++) {
                tmpdeck[pos] = deck[i];
                pos = (pos + n) % deck_size;
        }
        for (__int128 i=0; i<deck_size; i++) {
                deck[i] = tmpdeck[i];
        }
}
static __int128 index(__int128 i) {
        return deck[i];
}
static __int128 indexof(__int128 i) {
        for (__int128 j=0; j<deck_size; j++) {
                if (deck[j] == i) {
                        return j;
                }
        }
        return -1;
}

#else

// Macro implementing modulo operation in C
// (% is reminder operation)
// https://www.lemoda.net/c/modulo-operator/
#define MOD(a,b) ((((a)%(b))+(b))%(b))

static __int128 start;
static __int128 increment;

// https://en.wikipedia.org/wiki/Modular_exponentiation#Right-to-left_binary_method
__attribute__((const))
static __int128 modpow(__int128 base, __int128 exponent, __int128 modulus) {
        if (modulus == 1) {
                return 0;
        }

        __int128 result = 1;
        base = MOD(base, modulus);
        while (exponent > 0) {
                if (MOD(exponent, 2) == 1) {
                        result = MOD(result * base, modulus);
                }
                exponent = exponent >> 1;
                base = MOD(base * base, modulus);
        }
        return result;
}

static void factory_order(void) {
        start = 0;
        increment = 1;
}

static void deal_into_new_stack(__int128 _) {
        (void)_;
        increment = MOD(increment * -1, deck_size);
        start = MOD(start + increment, deck_size);
}

static void cut_cards(__int128 n) {
        start = MOD(start + increment * n, deck_size);
}

static void deal_with_increment(__int128 n) {
        increment = MOD(increment * modpow(n, deck_size-2, deck_size), deck_size);
}

static __int128 index(__int128 i) {
        return MOD(start + i * increment, deck_size);
}

static __int128 indexof(__int128 i) {
        for (__int128 j=0; j<deck_size; j++) {
                if (MOD(start + j * increment, deck_size) == i)
                        return j;
        }
        return -1;
}

#endif

struct command {
        void(*fun)(__int128);
        __int128 param;
};

static struct command parse_line(const char *const input, size_t *i) {
        struct command c;
        
        if (input[*i] == 'c') { // cut
                *i += 3;
                c.fun = cut_cards;
        } else if (input[*i] == 'd') {
                *i += 5;
                if (input[*i] == 'w') {
                        c.fun = deal_with_increment;
                } else if (input[*i] == 'i') {
                        c.fun = deal_into_new_stack;
                } else {
                        FAIL("parse error");
                }
                *i += 14;
        } else {
                FAIL("parse error");
        }
        
        c.param = 0;

        if (input[*i] == '\n') {
                return c;
        }
        ASSERT(input[*i] == ' ', "parse error: %d", input[*i]);
        ++*i;
        
        bool neg = false;
        if (input[*i] == '-') {
                neg = true;
                ++*i;
        }
        
        for (;; ++*i) {
                char ch = input[*i];
                if (ch == '\n') {
                        break;
                }
                c.param = c.param * 10 + ch - '0';
        }

        if (neg)
                c.param = -c.param;
        
        return c;
}

static struct command *parse_input(const char *const input, size_t *size) {
        struct command *result = NULL;
        size_t rs = 0;
        size_t ri = 0;

        for (size_t i=0;; i++) {
                if (input[i] == '\0') {
                        break;
                }
                
                if (ri >= rs) {
                        rs = rs > 0 ? rs * 2 : 32;
                        result = realloc(result, rs * sizeof *result);
                }
                
                result[ri++] = parse_line(input, &i);
                ASSERT(input[i] == '\n', "parse error");
        }

        result = realloc(result, ri * sizeof *result);
        *size = ri;
        return result;
}

static void solution1(const char *const input, char *const output) {
        size_t size;
        struct command *commands = parse_input(input, &size);

        deck_size = 10007;
#ifdef BRAINLET_MODE
        deck = malloc(deck_size * sizeof *deck);
        tmpdeck = malloc(deck_size * sizeof *tmpdeck);
        tmpdeck2 = malloc(deck_size * sizeof *tmpdeck2);
#endif

        factory_order();
        for (size_t i=0; i<size; i++) {
                commands[i].fun(commands[i].param);
        }

        (void)index;
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", (long)indexof(2019));
        free(commands);

#ifdef BRAINLET_MODE
        free(deck);
        free(tmpdeck);
        free(tmpdeck2);
#endif
}

static void solution2(const char *const input, char *const output) {
#ifdef BRAINLET_MODE
        FAIL("Not for brainlets sorry");
        // Non brainlet mode mercilessly and shamelessly copied from
        // https://www.reddit.com/r/adventofcode/comments/ee0rqi/2019_day_22_solutions/fbnkaju/
        // Also, didn't know there was a 128 integer implementation in gcc and clang
        // 128 bit integers are needed to avoid overflow, at least for my input
#endif
        
        size_t size;
        struct command *commands = parse_input(input, &size);

        deck_size = 119315717514047;

        factory_order();
        for (size_t i=0; i<size; i++) {
                commands[i].fun(commands[i].param);
        }

        __int128 start_diff = start;
        __int128 increment_mult = increment;

        increment = modpow(increment_mult, 101741582076661, deck_size);
        start = MOD(start_diff * (1 - modpow(increment_mult, 101741582076661, deck_size)), deck_size);
        start = MOD(start * modpow(MOD(1-increment_mult, deck_size), deck_size-2, deck_size), deck_size);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", (long)index(2020));
        free(commands);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
