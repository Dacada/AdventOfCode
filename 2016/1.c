#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define INDEX(x,y) (((y)+256)*512+((x)+256))

enum direction {
        NORTH=0,
        EAST=1,
        SOUTH=2,
        WEST=3
};

static void advance(int amount, enum direction dir, int *const x, int *const y) {
        switch (dir) {
        case NORTH:
                *y -= amount;
                break;
        case EAST:
                *x += amount;
                break;
        case SOUTH:
                *y += amount;
                break;
        case WEST:
                *x -= amount;
                break;
        }
}

static bool slow_advance(int amount, enum direction dir, int *const x, int *const y, bool *visited) {
        int dx=0, dy=0;
        switch (dir) {
        case NORTH:
                dy = -1;
                break;
        case EAST:
                dx = +1;
                break;
        case SOUTH:
                dy = +1;
                break;
        case WEST:
                dx = -1;
                break;
        }

        for (int i=0; i<amount; i++) {
                *x += dx;
                *y += dy;
                if (visited[INDEX(*x,*y)]) {
                        return true;
                }
                visited[INDEX(*x,*y)] = true;
        }
        return false;
}

static void solution(const char *input, char *const output, bool first_repeat) {
        enum direction direction = NORTH;
        int number = 0;
        int x=0,y=0;

        static bool visited[512*512];
        visited[INDEX(0,0)] = 0;
        
        while (*input != '\0') {
                switch (*input) {
                case 'R':
                        direction = (direction + 1) % 4;
                        input++;
                        break;
                case 'L':
                        direction = (direction - 1) % 4;
                        input++;
                        break;
                case ' ':
                case ',':
                case '\n':
                        input++;
                        break;
                default:
                        while (isdigit(*input)) {
                                number = number*10 + *input-'0';
                                input++;
                        }

                        if (first_repeat) {
                                if (slow_advance(number, direction, &x, &y, visited)) {
                                        goto end;
                                }
                        } else {
                                advance(number, direction, &x, &y);
                        }
                        number = 0;
                }
        }

end:
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", x+y);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, false);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, true);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
