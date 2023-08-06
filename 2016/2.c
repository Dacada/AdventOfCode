#include <aoclib.h>
#include <stdio.h>

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

static void solution1(const char *input, char *const output) {
        static const unsigned numpad[3][3] = {
                {1, 2, 3},
                {4, 5, 6},
                {7, 8, 9}
        };
        
        unsigned code = 0;
        int x=1,y=1;
        
        while (*input != '\0') {
                switch (*input) {
                case 'U':
                        y = MAX(0, y-1);
                        break;
                case 'D':
                        y = MIN(2, y+1);
                        break;
                case 'L':
                        x = MAX(0, x-1);
                        break;
                case 'R':
                        x = MIN(2, x+1);
                        break;
                case '\n':
                        code = code * 10 + numpad[y][x];
                        break;
                default:
                        FAIL("Unexpected input: %c", *input);
                }
                input++;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", code);
}

static void solution2(const char *input, char *const output) {
        static const char numpad[5][5] = {
                { 0 ,  0 , '1',  0 ,  0 },
                { 0 , '2', '3', '4',  0 },
                {'5', '6', '7', '8', '9'},
                { 0 , 'A', 'B', 'C',  0 },
                { 0 ,  0 , 'D',  0 ,  0 }
        };
        
        char code[8]={0};
        int i=0;
        int x=0,y=2;
        int nx,ny;
        
        while (*input != '\0') {
                nx = x;
                ny = y;
                
                switch (*input) {
                case 'U':
                        ny = MAX(0, y-1);
                        break;
                case 'D':
                        ny = MIN(4, y+1);
                        break;
                case 'L':
                        nx = MAX(0, x-1);
                        break;
                case 'R':
                        nx = MIN(4, x+1);
                        break;
                case '\n':
                        code[i++] = numpad[y][x];
                        break;
                default:
                        FAIL("Unexpected input: %c", *input);
                }

                if (numpad[nx][ny]) {
                        x = nx;
                        y = ny;
                }
                
                input++;
        }

        for (int j=i-1; j>=0; j--) {
                char tmp = code[j];
                code[j] = code[i-j-1];
                code[i-j-1] = tmp;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", code);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
