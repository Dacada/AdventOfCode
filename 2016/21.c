#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

enum operation_type {
        SWAP_POSITION,
        SWAP_LETTER,
        ROTATE_STEPS,
        ROTATE_BASED,
        REVERSE_POSITIONS,
        MOVE_POSITION,
};

struct operation {
        enum operation_type type;
        int x, y;
};

static int modulo(int a, unsigned b) {
        if (a < 0) {
                unsigned n = (-a)/b;
                a += b*(n+1);
        }
        return a % b;
}

static int read_number(const char **const input) {
        int n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }
        return n;
}

static int read_letter(const char **const input) {
        int c = **input;
        *input += 1;
        return c;
}

static struct operation read_line(const char **const input) {
        struct operation op = {0};

        int xpos = -1;
        int ypos = -1;
        bool nums;
        
        if (**input == 's') {
                *input += 5;
                if (**input == 'p') {
                        op.type = SWAP_POSITION;
                        nums = true;
                        xpos = 9;
                        ypos = 15;
                } else if (**input == 'l') {
                        op.type = SWAP_LETTER;
                        nums = false;
                        xpos = 7;
                        ypos = 13;
                } else {
                        ASSERT(false, "parse error");
                }
        } else if (**input == 'r') {
                *input += 1;
                if (**input == 'o') {
                        *input += 6;
                        if (**input == 'l' || **input == 'r') {
                                op.type = ROTATE_STEPS;
                                if (**input == 'r') {
                                        op.y = 1;
                                        xpos = 6;
                                } else {
                                        op.y = 0;
                                        xpos = 5;
                                }
                                nums = true;
                        } else if (**input == 'b') {
                                op.type = ROTATE_BASED;
                                xpos = 28;
                                nums = false;
                        } else {
                                ASSERT(false, "parse error");
                        }
                } else if (**input == 'e') {
                        op.type = REVERSE_POSITIONS;
                        xpos = 17;
                        ypos = 9;
                        nums = true;
                } else {
                        ASSERT(false, "parse error");
                }
        } else if (**input == 'm') {
                op.type = MOVE_POSITION;
                xpos = 14;
                ypos = 13;
                nums = true;
        } else {
                ASSERT(false, "parse error");
        }

        if (xpos > 0) {
                *input += xpos;
                if (nums) {
                        op.x = read_number(input);
                } else {
                        op.x = read_letter(input);
                }
        }
        if (ypos > 0) {
                *input += ypos;
                if (nums) {
                        op.y = read_number(input);
                } else {
                        op.y = read_letter(input);
                }
        }

        while (**input != '\n' && **input != '\0') {
                *input += 1;
        }
        if (**input == '\n') {
                *input += 1;
        }

        return op;
}

static struct operation *parse_input(const char *input, size_t *const count) {
        size_t size = 16;
        struct operation *ops = malloc(size * sizeof(*ops));

        while (*input != '\0') {
                if (*count >= size) {
                        size *= 2;
                        ops = realloc(ops, size * sizeof(*ops));
                }

                ops[*count] = read_line(&input);
                (*count)++;
        }

        return ops;
}

char password[] = "abcdefgh";
int letter_locations[] = {0,1,2,3,4,5,6,7};
const size_t passlen = sizeof(password)/sizeof(*password) - 1;

static void doswap(int x, int y, char l1, char l2) {
        password[y] = l1;
        password[x] = l2;
        letter_locations[l1-'a'] = y;
        letter_locations[l2-'a'] = x;
}

static void dorotate(int d) {
        for (unsigned i=0; i<passlen; i++) {;
                letter_locations[i] = modulo(letter_locations[i] + d, passlen);
                password[letter_locations[i]] = i + 'a';
        }
}

static void scramble(const struct operation *const ops, const size_t nops) {
        //DBG("%s\n", password);
        for (size_t i=0; i<nops; i++) {
                switch (ops[i].type) {
                case SWAP_POSITION:
                        //DBG("swap position %d with position %d", ops[i].x, ops[i].y);
                        {
                                int x = ops[i].x;
                                int y = ops[i].y;
                                char l1 = password[x];
                                char l2 = password[y];
                                doswap(x, y, l1, l2);
                        }
                        break;
                case SWAP_LETTER:
                        //DBG("swap letter %c with letter %c", ops[i].x, ops[i].y);
                        {
                                char l1 = ops[i].x;
                                char l2 = ops[i].y;
                                int x = letter_locations[l1-'a'];
                                int y = letter_locations[l2-'a'];
                                doswap(x, y, l1, l2);
                        }
                        break;
                case ROTATE_STEPS:
                        //DBG("rotate %s %d steps", (ops[i].y?"right":"left"), ops[i].x);
                        {
                                int d = ops[i].x;
                                if (!ops[i].y) {
                                        d = -d;   
                                }
                                dorotate(d);
                        }
                        break;
                case ROTATE_BASED:
                        //DBG("rotate based on position of letter %c", ops[i].x);
                        {
                                int d = letter_locations[ops[i].x-'a'];
                                if (d >= 4) {
                                        d++;
                                }
                                d++;
                                dorotate(d);
                        }
                        break;
                case REVERSE_POSITIONS:
                        //DBG("reverse positions %d through %d", ops[i].x, ops[i].y);
                        int a = ops[i].x;
                        int b = ops[i].y;
                        if (a > b) {
                                int tmp = a;
                                a = b;
                                b = tmp;
                        }
                        for (int j=0; j<=(b-a)/2; j++) {
                                int x = a + j;
                                int y = b - j;
                                char l1 = password[x];
                                char l2 = password[y];
                                doswap(x, y, l1, l2);
                        }
                        break;
                case MOVE_POSITION:
                        //DBG("move position %d to position %d", ops[i].x, ops[i].y);
                        {
                                int x = ops[i].x;
                                int y = ops[i].y;

                                int ilx = password[x]-'a';
                                char ly = letter_locations[password[y]-'a'];
                                
                                char tmp = password[x];
                                if (x > y) {
                                        // a b c d e f g h
                                        //   y       x
                                        
                                        // a b c d e   g h
                                        // a   b c d e g h
                                        // a f b c d e g h
                                        
                                        for (int j=x; j>y; j--) {
                                                password[j] = password[j-1];
                                                letter_locations[password[j-1]-'a'] = modulo(letter_locations[password[j-1]-'a']+1, passlen);
                                        }
                                } else {
                                        // a b c d e f g h
                                        //   x       y
                                        
                                        // a   c d e f g h
                                        // a c d e f   g h
                                        // a c d e f b g h
                                        
                                        for (int j=x; j<y; j++) {
                                                password[j] = password[j+1];
                                                letter_locations[password[j+1]-'a'] = modulo(letter_locations[password[j+1]-'a']-1, passlen);
                                        }
                                }
                                
                                password[y] = tmp;
                                letter_locations[ilx] = ly;
                        }
                        break;
                }

/*                 DBG("%s", password); */
/* #ifdef DEBUG */
/*                 fprintf(stderr, "                "); */
/*                 for (unsigned j=0; j<passlen; j++) { */
/*                         fprintf(stderr, "%d ", letter_locations[j]); */
/*                 } */
/*                 fprintf(stderr, "\n"); */
/* #endif */
        }
}

static void solution1(const char *const input, char *const output) {
        size_t nops = 0;
        struct operation *ops = parse_input(input, &nops);

        scramble(ops, nops);

        strcpy(output, password);
        free(ops);
}

struct scramble_permutation_args {
        const size_t nops;
        const struct operation *const ops;
        char result[9];
};

static void scramble_permutation(int *const pwd, void *const vargs) {
        for (int i=0; i<8; i++) {
                password[i] = pwd[i] + 'a';
                letter_locations[pwd[i]] = i;
        }
        struct scramble_permutation_args *args = vargs;
        
        scramble(args->ops, args->nops);

        if (strcmp(password, "fbgdceah") == 0) {
                for (int i=0; i<8; i++) {
                        args->result[i] = pwd[i] + 'a';
                }
        }
}

static void solution2(const char *const input, char *const output) {
        size_t nops = 0;
        struct operation *ops = parse_input(input, &nops);

        struct scramble_permutation_args args = {
                .nops = nops,
                .ops = ops
        };
        args.result[8] = '\0';

        int pwd[8] = {0, 1, 2, 3, 4, 5, 6, 7};
        aoc_permute(pwd, 8, scramble_permutation, &args);

        strcpy(output, args.result);
        free(ops);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
