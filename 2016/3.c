#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

static bool isvalid(unsigned triangle[3]) {
        return triangle[0] + triangle[1] > triangle[2] &&
                triangle[0] + triangle[2] > triangle[1] &&
                triangle[1] + triangle[2] > triangle[0];
}

static void solution1(const char *input, char *const output) {
        unsigned result = 0;
        
        while (*input) {
                unsigned triangle[3]={0};
                for (int i=0; i<3; i++) {
                        while (isspace(*input)) {
                                input++;
                        }
                        while (isdigit(*input)) {
                                triangle[i] = triangle[i]*10+*input-'0';
                                input++;
                        }
                }

                if (isvalid(triangle)) {
                        result++;
                }

                ASSERT(*input == '\n', "Unexpected input");
                input++;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

static void solution2(const char *input, char *const output) {
        unsigned result = 0;

        while (*input) {
                unsigned triangles[3][3]={0};
                for (int i=0; i<3; i++) {
                        for (int j=0; j<3; j++) {
                                while (isspace(*input)) {
                                        input++;
                                }
                                while (isdigit(*input)) {
                                        triangles[j][i] = triangles[j][i]*10+*input-'0';
                                        input++;
                                }
                        }

                        ASSERT(*input == '\n', "Unexpected input");
                        input++;
                }

                for (int i=0; i<3; i++) {
                        if (isvalid(triangles[i])) {
                                result++;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
