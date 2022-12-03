#include <aoclib.h>
#include <stdio.h>

#define ROCK 0
#define PAPER 1
#define SCISSORS 2

#define LOSE 0
#define DRAW 1
#define WIN 2

// the next two tables codify how
// SHAPE will OUTCOME against SHAPE

// what is the outcome for shape i against shape j
// i will rps[i][j] against j
const int rps[3][3] = {
        //rock  paper scissors
        { DRAW, LOSE, WIN  }, // rock
        { WIN , DRAW, LOSE }, // paper
        { LOSE, WIN , DRAW }, // scissors
};

// what shape will have a certain outcome i against shape j
// invrps[i][j] will i against j
const int invrps[3][3] = {
        //rock      paper     scissors
        { SCISSORS, ROCK    , PAPER    }, // lose
        { ROCK    , PAPER   , SCISSORS }, // draw
        { PAPER   , SCISSORS, ROCK     }, // win
};

const int score_shape[3] = { 1, 2, 3 };
const int score_outcome[3] = { 0, 3, 6 };

static int score_round_1(int opponent, int self) {
        return score_shape[self] + score_outcome[rps[self][opponent]];
}

static int score_round_2(int opponent, int self) {
        return score_shape[invrps[self][opponent]] + score_outcome[self];
}

static int parse_input(const char **const input) {
        if (**input == '\0') {
                return -1;
        }
        
        char opponent = **input;
        *input += 1;
        
        ASSERT(**input == ' ', "parse error");
        *input += 1;
        
        char self = **input;
        *input += 1;

        if (**input != '\0') {
                ASSERT(**input == '\n', "parse error");
                *input += 1;
        }

        return (opponent - 'A') * 3 + (self - 'X');
}

static void solution(const char *input, char *const output, int(*score_round)(int,int)) {
        int round;
        int score = 0;
        while ((round = parse_input(&input)) >= 0) {
                score += score_round(round / 3, round % 3);
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", score);
}

static void solution1(const char *input, char *const output) {
        solution(input, output, score_round_1);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, score_round_2);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
