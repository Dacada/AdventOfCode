#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define MAXSIZE 30000000

static int parse_int(const char **input) {
        char c;
        unsigned n = 0;
        while (isdigit(c = **input)) {
                n = n*10+c-'0';
                *input += 1;
        }
        return n;
}

static unsigned parse(const char *input, int *const numbers, int *const last) {
        unsigned turn = 0;
        for (; *input != '\0' && *input != '\n'; input++) {
                turn++;
                if (*last >= 0) {
                        numbers[*last] = turn - 1;
                }
                int number = parse_int(&input);
                DBG("The %uth number spoken is a starting number, %d", turn, number);
                *last = number;

                if (*input == '\0') {
                        break;
                }
                ASSERT(*input == ',' || *input == '\n', "parse error");
        }

        return turn+1;
}

static void solution(const char *const input, char *const output, unsigned goal) {
        static int numbers[MAXSIZE];
        int last = -1;
        
        unsigned turn = parse(input, numbers, &last);
        for (; turn<=goal; turn++) {
                unsigned prev_turn = turn - 1;
                
                unsigned number;
                if (numbers[last] == 0) {
                        number = 0;
                        DBG("The %uth number spoken is 0 as the last one, %d, had not been seen before",
                            turn, last);
                } else {
                        number = prev_turn - numbers[last];
                        DBG("The %uth number spoken is %u as the last one, %d, "
                            "has been seen on turn %d and %u - %d = %u",
                            turn, number, last, numbers[last], turn-1, numbers[last], number);
                }

                numbers[last] = prev_turn;
                last = number;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", last);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 2020);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 30000000);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
