#include <aoclib.h>
#include <stdio.h>

static int parse_int(const char *const text, size_t *const idx) {
        int n = 0;
        int i;
        for (i=0; i<10; i++) {
                char c = text[*idx+i];
                if (c < '0' || c > '9') {
                        break;
                }
                n = n*10+c-'0';
        }
        *idx += i;
        return n;
}

static bool password_valid_1(const char *const text, size_t *const idx, int min, int max, char letter) {
        size_t i;
        int letter_count = 0;
        for (i=0;; i++) {
                char c = text[*idx+i];
                if (c < 'a' || c > 'z') {
                        break;
                }
                if (c == letter) {
                        letter_count++;
                }
        }
        *idx += i;
        return letter_count >= min && letter_count <= max;
}

static bool password_valid_2(const char *const text, size_t *const idx, int i1, int i2, char letter) {
        bool is1 = text[*idx+i1-1] == letter;
        bool is2 = text[*idx+i2-1] == letter;
        while (text[*idx] >= 'a' && text[*idx] <= 'z') {
                (*idx)++;
        }
        return is1 != is2;
}

static void solution(const char *const input, char *const output,
                     bool(*password_valid)(const char*, size_t*, int, int, char)) {
        int count = 0;
        for (size_t i=0;; i++) {
                if (input[i] == '\0') {
                        break;
                }
                int a = parse_int(input, &i);
                ASSERT(input[i] == '-', "invalid input");
                i++;
                int b = parse_int(input, &i);
                ASSERT(input[i] == ' ', "invalid input");
                i++;
                char letter = input[i++];
                ASSERT(input[i] == ':', "invalid input");
                i++;
                ASSERT(input[i] == ' ', "invalid input");
                i++;
                if (password_valid(input, &i, a, b, letter)) {
                        count++;
                }
                ASSERT(input[i] == '\n', "invalid input");
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, password_valid_1);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, password_valid_2);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
