#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct vector {
        int x;
        int y;
};

struct rectangle {
        struct vector ul;
        struct vector dr;
};

static void assert_msg(const char **const input, const char *const msg) {
        size_t len = strlen(msg);
        ASSERT(strncmp(*input, msg, len) == 0, "parse error");
        *input += len;
}

static int parse_number(const char **const input) {
        bool neg = false;
        if (**input == '-') {
                neg = true;
                *input += 1;
        }

        int n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }

        if (neg) {
                n = -n;
        }
        return n;
}

__attribute__((pure))
static struct rectangle parse_input(const char *input) {
        struct rectangle res;
        assert_msg(&input, "target area: x=");
        res.ul.x = parse_number(&input);
        assert_msg(&input, "..");
        res.dr.x = parse_number(&input);
        assert_msg(&input, ", y=");
        res.dr.y = parse_number(&input);
        assert_msg(&input, "..");
        res.ul.y = parse_number(&input);
        ASSERT(*input == '\n' || *input == '\0', "parse error");
        return res;
}

#define TRI(n) ((n)*((n)+1)/2)

static void solution1(const char *const input, char *const output) {
        struct rectangle targetArea = parse_input(input);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", TRI(-targetArea.dr.y-1));
}

static void solution2(const char *const input, char *const output) {
        struct rectangle targetArea = parse_input(input);

        int vel = 0;
        int min_x_vel=0;
        int distance = 0;
        while (distance < targetArea.dr.x){
                vel++;
                if ( (distance = TRI(vel)) > targetArea.ul.x){
                        min_x_vel = vel;
                        break;
                }
        }

        int res = 0;
        for (int i=min_x_vel; i<=targetArea.dr.x; i++){
                for (int j=targetArea.dr.y; j<=-targetArea.dr.y-1; j++){
                        int xpos = 0;
                        int ypos = 0;
                        int xvel = i;
                        int yvel = j;

                        while (xpos <= targetArea.dr.x && ypos >= targetArea.dr.y){
                                xpos += xvel;
                                ypos += yvel;
                                if (xvel > 0){ xvel -= 1; }
                                yvel -= 1;
                                
                                if (xpos >= targetArea.ul.x && xpos <= targetArea.dr.x && ypos >= targetArea.dr.y && ypos <= targetArea.ul.y){
                                        res++;
                                        break;
                                }
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
