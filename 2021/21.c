#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static unsigned parse_number(const char **const input) {
        ASSERT(isdigit(**input), "parse error");
        unsigned res = 0;
        while (isdigit(**input)) {
                res *= 10;
                res += **input - '0';
                *input += 1;
        }
        return res;
}

static void assert_msg(const char **const input, const char *const msg) {
        size_t len = strlen(msg);
        ASSERT(strncmp(*input, msg, len) == 0, "parse error");
        *input += len;
}

static void parse_input(const char *input, unsigned *const p1, unsigned *const p2) {
        assert_msg(&input, "Player 1 starting position: ");
        *p1 = parse_number(&input);
        ASSERT(*p1 > 0 && *p1 <= 10, "parse error");
        *p1 -= 1;
        ASSERT(*input == '\n', "parse error");
        input += 1;
        
        assert_msg(&input, "Player 2 starting position: ");
        *p2 = parse_number(&input);
        ASSERT(*p2 > 0 && *p2 <= 10, "parse error");
        *p2 -= 1;
        ASSERT(*input == '\n', "parse error");
        input += 1;
}

static void solution1(const char *const input, char *const output) {
        // Easy and slow way
        
        unsigned position[2];
        parse_input(input, position, position+1);

        unsigned rolls=0;
        unsigned points[2] = {0,0};

        unsigned current = 0;
        while (points[0] < 1000 && points[1] < 1000) {
                for (int i=0; i<3; i++) {
                        position[current] += rolls%100 + 1;
                        rolls++;
                }
                position[current] %= 10;
                points[current] += position[current] + 1;
                current = (current + 1) % 2;
        }

        unsigned loser;
        if (points[0] >= 1000) {
                loser = 1;
        } else {
                loser = 0;
        }

        DBG("Player %u loses. Player 1 has %u points. Player 2 has %u points. The dice has been rolled %u times.", loser+1, points[0], points[1], rolls);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", points[loser]*rolls);
}

static void solution2(const char *const input, char *const output) {
        // On each trio of rolls, the player gets...
        //     - 3 in 1 universes
        //     - 4 in 3 universes
        //     - 5 in 6 universes
        //     - 6 in 7 universes
        //     - 7 in 6 universes
        //     - 8 in 3 universes
        //     - 9 in 1 universes
        const unsigned roll_counts[] = {0,0,0,1,3,6,7,6,3,1};
        
        // Keep track of every possible combination of position and number of
        // points for each player
        
        // state[position player 1][points player 1][position player 2][points player 2] = number of universes
        
        static unsigned long state[10][21][10][21];
        memset(state, 0, sizeof(state));
        
        unsigned p1, p2;
        parse_input(input, &p1, &p2);
        /* p1 = 4-1; */
        /* p2 = 8-1; */
        state[p1][0][p2][0] = 1;

        // Now play the game simultaneuosly in all universes
        
        bool done = false;
        unsigned long winners[2] = {0,0};
        while (!done) {
                done = true;
                
                static unsigned long new_state[10][21][10][21];
                memset(new_state, 0, sizeof(new_state));

                // Advance state, creating new universes
                for (unsigned pos1=0; pos1<10; pos1++) {
                        for (unsigned pts1=0; pts1<=20; pts1++) {
                                for (unsigned pos2=0; pos2<10; pos2++) {
                                        for (unsigned pts2=0; pts2<=20; pts2++) {
                                                unsigned long universes = state[pos1][pts1][pos2][pts2];
                                                if (universes == 0) {
                                                        continue;
                                                }
                                                
                                                // ends only when there's no more universes
                                                done = false;
                                                
                                                for (unsigned roll1=3; roll1<=9; roll1++) {
                                                        unsigned long universes_after_p1_rolls = universes * roll_counts[roll1];
                                                        unsigned new_pos1 = (pos1 + roll1) % 10;
                                                        unsigned new_pts1 = pts1 + new_pos1 + 1;
                                                        if (new_pts1 >= 21) {
                                                                winners[0] += universes_after_p1_rolls;
                                                                continue;
                                                        }
                                                                
                                                        for (unsigned roll2=3; roll2<=9; roll2++) {
                                                                unsigned long universes_after_p2_rolls = universes_after_p1_rolls * roll_counts[roll2];
                                                                unsigned new_pos2 = (pos2 + roll2) % 10;
                                                                unsigned new_pts2 = pts2 + new_pos2 + 1;
                                                                if (new_pts2 >= 21) {
                                                                        winners[1] += universes_after_p2_rolls;
                                                                        continue;
                                                                }

                                                                new_state[new_pos1][new_pts1][new_pos2][new_pts2] += universes_after_p2_rolls;
                                                        }
                                                }
                                        }
                                }
                        }
                }

                memcpy(state, new_state, sizeof(state));
        }

        DBG("Player 1: %lu", winners[0]);
        DBG("Player 2: %lu", winners[1]);
        unsigned long solution = winners[0];
        if (winners[1] > solution) {
                solution = winners[1];
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", solution);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
