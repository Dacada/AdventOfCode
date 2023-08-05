#include <aoclib.h>
#include <stdio.h>

static long parse_int(const char **input) {
        long r = 0;
        for (;;) {
                bool done = false;
                long d;
                switch(**input) {
                case '=':
                        d = -2;
                        break;
                case '-':
                        d = -1;
                        break;
                case '0':
                        d = 0;
                        break;
                case '1':
                        d = 1;
                        break;
                case '2':
                        d = 2;
                        break;
                default:
                        done = true;
                        break;
                }
                if (done) {
                        break;
                }

                r *= 5;
                r += d;
                *input += 1;
        }

        return r;
}

static void write_int(long n, char *buff) {
        ASSERT(n > 0, "won't convert negatives");
        const char glyphs[] = {'=', '-', '0', '1', '2'};
        int len = 0;
        while (n > 0) {
                long d = n % 5;

                d += 2;
                if (d >= 5) {
                        d -= 5;
                        n += 5;
                }

                n /= 5;

                buff[len++] = glyphs[d];
        }
        buff[len] = '\0';

        // reverse result
        for (int i=0; i<len/2; i++) {
                int j = len - i - 1;
                char tmp = buff[i];
                buff[i] = buff[j];
                buff[j] = tmp;
        }
}

static long parse_input(const char *input, long **numbers) {
        int len = 0;
        int cap = 8;
        *numbers = malloc(sizeof(*numbers)*cap);
        
        while (*input != '\0') {
                if (len >= cap) {
                        cap *= 2;
                        *numbers = realloc(*numbers, sizeof(*numbers)*cap);
                }
                (*numbers)[len++] = parse_int(&input);
                while (*input == '\n') {
                        input++;
                }
        }
        
        return len;
}

static void solution1(const char *const input, char *const output) {
        long *numbers;
        int len = parse_input(input, &numbers);

        long total = 0;
        for (int i=0; i<len; i++) {
                long n = numbers[i];
                DBG("%ld", n);
                total += n;
        }
        DBG("total: %ld", total);
        
        char result[64];
        write_int(total, result);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", result);
        free(numbers);
}

static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "CHRISTMAS :)");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
