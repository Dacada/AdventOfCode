#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

static const char *next_number(const char *input, unsigned *const num) {
        while (!isdigit(*input)) {
                input++;
        }
        
        *num = 0;
        while (isdigit(*input)) {
                *num *= 10;
                *num += *input - '0';
                input++;
        }

        return input;
}

static void parse_input(const char *input, unsigned *const a, unsigned *const b) {
        next_number(next_number(input, a), b);
}

static unsigned table_index(unsigned a, unsigned b) {
        /*
         * We want to find out what's at column a, row b.
         *
         * If we go backwards (a-1) elements we will end up at column
         * 1, row b+a-1.
         *
         * So the corresponding index will be (a-1) + whatever is at
         * column 1, row b+a-1.
         *
         * Now if we're at column 1, row n. Whatever is there will be
         * whatever was at column 1, row (n-1) + (n-1) since each
         * diagonal is the size of the corresponding row number.
         *
         * We find the recurrence (1,n) = (n-1) + (1,(n-1)) =
         * (n-1)+(n-2) + (1,(n-1)) = ... = (n-1)+(n-2)+...+1 + (1,1) =
         * sum(1 to n-1) + 1
         *
         * We know that the sum of all numbers from 1 to m is:
         * n*(n+1)/2.
         *
         * So putting it all together: (a,b) = (a-1) + sum(1 to
         * (1,b+a-1)-1) + 1 which simplifies to the expression below
         * if we use the previous formula to compute the sum of
         * numbers from 1 to n.
         */
        return a+(a+b-2)*(a+b-1)/2;
}

static unsigned lehmer(unsigned long x, unsigned a, unsigned m, unsigned i) {
        while (i--) {
                x = x * a % m;
        }
        return x;
}

static void solution1(const char *const input, char *const output) {
        unsigned row, column;
        parse_input(input, &row, &column);

        unsigned result = lehmer(20151125, 252533, 33554393, table_index(column, row) - 1);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "C H R I S T M A S");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
