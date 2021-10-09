#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define MAXDUB (1<<21)
#define CHARNEG(b) ((b)=='0'?'1':'0')

/*

01000100010010111
01000100010010111000010110111011101
01000100010010111000010110111011101001000100010010111100010110111011101
01000100010010111000010110111011101001000100010010111100010110111011101001000100010010111000010110111011101101000100010010111100010110111011101
01000100010010111000010110111011101001000100010010111100010110111011101001000100010010111000010110111011101101000100010010111100010110111011101001000100010010111000010110111011101001000100010010111100010110111011101101000100010010111000010110111011101101000100010010111100010110111011101
01000100010010111000010110111011101001000100010010111100010110111011101001000100010010111000010110111011101101000100010010111100010110111011101001000100010010111000010110111011101001000100010010111100010110111011101101000100010010111000010110111011101101000100010010111100



01000100010010111
0 -> a, 0
00010110111011101
0 -> b, 0
01000100010010111
1 -> c, ~a = ~0 = 1
00010110111011101
0 -> d, 0
01000100010010111
0 -> e, ~c = ~~a = 0
00010110111011101
1 -> f, ~b = 1
01000100010010111
1 -> g, ~a = 1
00010110111011101
0 -> h, 0
01000100010010111
0 -> i, ~g = ~~a = 0
00010110111011101
0 -> j, ~f = ~~b = 0
01000100010010111
1 -> k, ~e = ~~c = ~~~a = 1
00010110111011101
1 -> l, ~d = 1
01000100010010111
0 -> m, ~c = ~~a = 0
00010110111011101
1 -> n, ~b = 1
01000100010010111
1 -> o, ~a = 1
00


0 1 2       3 4       5       6       7 8       9       10      11      12      13      14
0 0 !f(0)=1 0 !f(2)=0 !f(1)=1 !f(0)=1 0 !f(6)=0 !f(5)=0 !f(4)=1 !f(3)=1 !f(2)=0 !f(1)=1 !f(0)=1


easy to calculate previous one if we know where we got last one from


----
checksum
----


01000100010010111000010110111011101001000100010010111100010110111011101001000100010010111000010110111011101101000100010010111100010110111011101001000100010010111000010110111011101001000100010010111100010110111011101101000100010010111000010110111011101101000100010010111100
0101010101000101000101010111000101000101010101000101010101010111000101000101010101000101000101010111000101010101010101000101010101010111
00000100100001100100000100000001100100000100100001100000000100000001
1101010001101110001101010011101110
10010010110011010


01000100010010111 0 00010110111011101 0 01000100010010111 1 00010110111011101 0 01000100010010111 0 00010110111011101 1 01000100010010111 1 00010110111011101 0 01000100010010111 0 00010110111011101 0 01000100010010111 1 00010110111011101 1 01000100010010111 0 00010110111011101 1 01000100010010111 1 00
 0 1 0 1 0 1 0 1  0  1 0 0 0 1 0 1 0  0  0 1 0 1 0 1 0 1  1  1 0 0 0 1 0 1 0  0  0 1 0 1 0 1 0 1  0  1 0 0 0 1 0 1 0  1  0 1 0 1 0 1 0 1  1  1 0 0 0 1 0 1 0  0  0 1 0 1 0 1 0 1  0  1 0 0 0 1 0 1 0  0  0 1 0 1 0 1 0 1  1  1 0 0 0 1 0 1 0  1  0 1 0 1 0 1 0 1  0  1 0 0 0 1 0 1 0  1  0 1 0 1 0 1 0 1  1  1
   0   0   0   0     0   1   0   0    1    0   0   0   0     1   1   0   0    1    0   0   0   0     0   1   0   0    0    0   0   0   0     1   1   0   0    1    0   0   0   0     0   1   0   0    1    0   0   0   0     1   1   0   0    0    0   0   0   0     0   1   0   0    0    0   0   0   0     1
       1       1         0       1         0       1         0       0        0        1       1         0       1         1       1         0       0        0        1       1         0       1         0       1         0       0        1        1       1         0       1         1       1         0
               1                 0                 0                 1                 0                 0                 1                 0                1                1                 0                 0                 1                 1                 0                 1                 0



 */

static unsigned linelen(const char *const str) {
        for (unsigned i=0;; i++) {
                if (str[i] == '\n' || str[i] == '\0') {
                        return i;
                }
        }
}

static void reverse_and_negate(const char *const input, char *const output, const unsigned len) {
        for (unsigned i=0; i<len; i++) {
                output[i] = input[len-i-1] == '0' ? '1' : '0';
        }
}

struct checksum {
        char *memory;
        unsigned levels;
        
        char *result;
        unsigned result_idx;
        unsigned size;
};
static void checksum_init(struct checksum *const chk, const unsigned levels, const unsigned size) {
        chk->result = malloc((size+1)*sizeof(char));
        chk->result[size] = '\0';
        chk->result_idx = 0;
        chk->size = size;
        
        chk->memory = malloc(levels*sizeof(char));
        chk->levels = levels;
        memset(chk->memory, 0, levels*sizeof(char));
}
#define checksum_step(a, b) ((a)==(b)?'1':'0')
static void checksum_add(struct checksum *const chk, char element) {
        char *mem = chk->memory;

        for (unsigned level=0; level<chk->levels; level++) {
                if (*mem == '\0') {
                        *mem = element;
                        element = '\0';
                        break;
                }
                
                element = checksum_step(element, *mem);
                *mem = '\0';
                mem++;
        }

        if (element != '\0') {
                ASSERT(chk->result_idx < chk->size, "result buffer overrun");
                chk->result[chk->result_idx] = element;
                chk->result_idx++;
        }
}

static void checksum_free(struct checksum *const chk) {
        free(chk->result);
        free(chk->memory);
}

struct interbit {
        char bits[MAXDUB];
        unsigned i;
        bool prev_was_zero;
        unsigned revneg;
};
static void interbit_init(struct interbit *const bit) {
        bit->i = 0;
}
static char interbit_next(struct interbit *const bit) {
        char res;

        bool from_hist = false;
        if (bit->i == 0) {
                res = '0';
                bit->prev_was_zero = false;
                bit->revneg = 0;
        } else {
                if (bit->prev_was_zero) {
                        bit->prev_was_zero = false;
                        bit->revneg = bit->i - 2;
                        from_hist = true;
                } else {
                        if (bit->revneg == 0) {
                                res = '0';
                                bit->prev_was_zero = true;
                        } else {
                                bit->revneg--;
                                from_hist = true;
                        }
                }
        }
        if (from_hist) {
                if (bit->revneg >= MAXDUB) {
                        FAIL("requested size too big");
                }
                
                res = bit->bits[bit->revneg];
                if (res == '0') {
                        res = '1';
                } else if (res == '1') {
                        res = '0';
                } else {
                        FAIL("weird stuff");
                }
        }

        if (bit->i >= MAXDUB) {
                FAIL("requested size too big");
        }
        
        bit->bits[bit->i] = res;
        bit->i++;
        return res;
}

static void solution(const char *const input, char *const output, const unsigned size) {
        unsigned input_len = linelen(input);
        
        char *reversed_input = malloc(input_len*sizeof(char));
        reverse_and_negate(input, reversed_input, input_len);
        
        const char *current = input;
        const char *other = reversed_input;

        static struct checksum chk;
        unsigned checksum_size = size;
        unsigned checksum_levels = 0;
        while (checksum_size % 2 == 0) {
                checksum_size /= 2;
                checksum_levels++;
        }
        checksum_init(&chk, checksum_levels, checksum_size);

        static struct interbit bit;
        interbit_init(&bit);

        unsigned total_size = 0;
        while (total_size < size) {
                for (unsigned i=0; i<input_len && total_size < size; i++) {
                        checksum_add(&chk, current[i]);
                        total_size++;
                }
                if (total_size < size) {
                        char c = interbit_next(&bit);
                        checksum_add(&chk, c);
                        total_size++;
                }
                const char *tmp = current;
                current = other;
                other = tmp;
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", chk.result);

        free(reversed_input);
        checksum_free(&chk);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 272);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 35651584);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
