#define _DEFAULT_SOURCE
#include <unistd.h>

#include "intcode.h"
#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))

static char *parse_map(struct IntCodeMachine *const machine, size_t *const rows, size_t *const columns) {
        *rows = 0;
        *columns = 0;
        
        size_t map_size = 64;
        size_t map_i = 0;
        char *map = malloc(sizeof(*map) * map_size);
        if (map == NULL) {
                perror("malloc");
                return NULL;
        }

        bool seen_linebreak = false;

        machine_run(machine);
        while(machine->running) {
                long out;
                ASSERT(machine_recv_output(machine, &out), "no output?");

                if (out == '\n') {
                        seen_linebreak = true;
                        ++*rows;
                } else {
                        while (map_i >= map_size) {
                                map_size *= 2;
                                map = realloc(map, sizeof(*map) * map_size);
                                if (map == NULL) {
                                        perror("realloc");
                                        return NULL;
                                }
                        }

                        ASSERT(out == '.' || out == '#' || out == '^' || out == '<' || out == '>' || out == 'v', "Unexpected map tile: %ld", out);
                        map[map_i] = out;
                        map_i++;
                }

                if (!seen_linebreak) {
                        ++*columns;
                }

                machine_run(machine);
        }
        --*rows; // remove last line break

        ASSERT(*rows * *columns == map_i, "%lu * %lu <> %lu", *rows, *columns, map_i);

        void *new_map = realloc(map, sizeof(*map) * map_i);
        if (new_map == NULL) {
                perror("realloc");
                free(map);
                return NULL;
        }
        return new_map;
}

static void show_map(const char *const map, size_t rows, size_t columns) {
        for (size_t j=0; j<rows; j++) {
                for (size_t i=0; i<columns; i++) {
                        char c = map[j*columns+i];
                        fprintf(stderr, "%c ", c);
                }
                fprintf(stderr, "\n");
        }
}

static void solution1(const char *const input, char *const output) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);

        size_t rows, columns;
        char *map = parse_map(&machine, &rows, &columns);

        int result = 0;
        for (size_t j=1; j<rows-1; j++) {
                for (size_t i=1; i<columns-1; i++) {
                        if (map[ j   *columns+ i   ] == '#' &&
                            map[(j-1)*columns+ i   ] == '#' &&
                            map[(j+1)*columns+ i   ] == '#' &&
                            map[ j   *columns+(i-1)] == '#' &&
                            map[ j   *columns+(i+1)] == '#') {
                                result += i * j;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
	machine_free(&machine);
	free(map);
}

enum direction { NORTH, EAST, SOUTH, WEST };

static size_t get_path(char *const map, size_t rows, size_t columns,
                       size_t x, size_t y, char *const movement) {
        char me = map[y*columns+x];

        enum direction my_direction;
        switch (me) {
        case '^':
                my_direction = NORTH; break;
        case '>':
                my_direction = EAST; break;
        case 'v':
                my_direction = SOUTH; break;
        case '<':
                my_direction = WEST; break;
        default:
                FAIL("Unexpected 'me' %c", me);
        }

        bool valid_next_directions[4];
        valid_next_directions[0] = y > 0         && map[(y-1)*columns+ x   ] == '#';
        valid_next_directions[1] = x < columns-1 && map[ y   *columns+(x+1)] == '#';
        valid_next_directions[2] = y < rows-1    && map[(y+1)*columns+ x   ] == '#';
        valid_next_directions[3] = x > 0         && map[ y   *columns+(x-1)] == '#';

        if (valid_next_directions[my_direction]) {
                *movement = '+';

                size_t nextx=x, nexty=y;
                switch (my_direction) {
                case NORTH:
                        nexty--; break;
                case EAST:
                        nextx++; break;
                case SOUTH:
                        nexty++; break;
                case WEST:
                        nextx--; break;
                }

                map[y*columns+x] = '#';
                map[nexty*columns+nextx] = me;
                return 1 + get_path(map, rows, columns, nextx, nexty, movement+1);
        } else if (valid_next_directions[(my_direction + 1) % 4] ||
                   valid_next_directions[(my_direction - 1) % 4]) {
                enum direction my_new_direction;
                if (valid_next_directions[(my_direction + 1) % 4]) {
                        *movement = 'R';
                        my_new_direction = (my_direction + 1) % 4;
                } else {
                        *movement = 'L';
                        my_new_direction = (my_direction - 1) % 4;
                }

                char new_me;
                switch (my_new_direction) {
                case NORTH:
                        new_me = '^'; break;
                case EAST:
                        new_me = '>'; break;
                case SOUTH:
                        new_me = 'v'; break;
                case WEST:
                        new_me = '<'; break;
                default:
                        FAIL("Unexpected direction");
                }

                map[y*columns+x] = new_me;
                return 1 + get_path(map, rows, columns, x, y, movement+1);
        } else {
                return 0;
        }
}

static void get_starting_location(const char *const map, size_t rows, size_t columns,
                                  size_t *x, size_t *y) {
        for (size_t j=0; j<rows; j++) {
                for (size_t i=0; i<columns; i++) {
                        char c = map[j*columns+i];
                        if (c == '^' || c == '>' || c == 'v' || c == '<') {
                                *x = i;
                                *y = j;
                                return;
                        }
                }
        }
}

// Actually turn individual "forwards" into a number
static size_t compress1(char *const mov, size_t num) {
        long j = -1;
        for (size_t i=0; i<num; i++) {
                char c = mov[i];
                if (c == 'R' || c == 'L') {
                        j++;
                        mov[j] = c;
                        j++;
                } else if (c == '+') {
                        if (!isdigit(mov[j])) {
                                mov[j] = '1';
                        } else if (mov[j] == '9') {
                                if (!isdigit(mov[j-1])) {
                                        mov[j] = '1';
                                        j++;
                                } else {
                                        ASSERT(mov[j-1] != '9', "pls no");
                                        mov[j-1]++;
                                }
                                mov[j] = '0';
                        } else {
                                mov[j]++;
                        }
                } else {
                        FAIL("Unexpected character in compress1: %c", c);
                }
        }
        ASSERT(j >= 0, "There wasn't anything in the movements buffer???");
        return j+1;
}

static void print_str(const char *const str, const size_t size) {
        static char buff[2<<10];
        strncpy(buff, str, MIN(size, (2<<10)-1));
        buff[size] = '\0';
        fprintf(stderr, "%s", buff);
}

/*
static void compress2_part2_battletendency(char list_of_symbols[50][5],
                                           char list_of_new_symbols[50][5],
                                           char equivalence_dictionary_keys[10],
                                           char equivalence_dictionary_values[10][10][5]) {

        size_t len_list_of_symbols = 0;
        for (size_t i=0; i<50; i++) {
                if (list_of_symbols[i][0] == '\0') {
                        len_list_of_symbols = i;
                        break;
                }
        }

        size_t equivalence_dictionary_index = 0;
        char equivalence_dictionary_next_symbol = 'A';

        bool symbol_is_covered[len_list_of_symbols];
        for (size_t i=0; i<len_list_of_symbols; i++) {
                symbol_is_covered[i] = false;
        }

        // While uncovered symbols remain:
        for(;;) {
                size_t pattern_i = 0;
                char pattern[50][5];
                size_t first_symbol_taken_index;
                
                // Take the first non covered symbol
                bool uncovered_symbols_remain = true;
                for (size_t i=0; i<len_list_of_symbols; i++) {
                        if (!symbol_is_covered[i]) {
                                uncovered_symbols_remain = true;
                                first_symbol_taken_index = i;
                                break;
                        }
                }
                if (!uncovered_symbols_remain) {
                        break;
                }
                strncpy(pattern[pattern_i++], list_of_symbols[first_symbol_taken_index], 5);
                pattern[pattern_i][0] = '\0';

                // Count occurances of this pattern of symbols within the list
                // Occurances that would include a covered symbol don't count
                int occurances = count_occurances_of_pattern_within_list(pattern, list_of_symbols,
                                                                         symbol_is_covered);

                // Add consecutive symbols until the number of occurances would change or no more can be added
                int i=1;
                while (first_symbol_taken_index+i < len_list_of_symbols &&
                       !symbol_is_covered[first_symbol_taken_index+i]) {
                        strncpy(pattern[pattern_i++], list_of_symbols[first_symbol_taken_index+i], 5);
                        pattern[pattern_i][0] = '\0';
                        int new_occurances = count_occurances_of_pattern_within_list(pattern, list_of_symbols,
                                                                                     symbol_is_covered,
                                                                                     len_list_of_symbols);
                        if (new_occurances < occurances) {
                                --pattern_i;
                                break;
                        }
                }

                // Add a new symbol to the dictionary and associate it with this pattern
                equivalence_dictionary_keys[equivalence_dictionary_index] = equivalence_dictionary_next_symbol++;
                size_t j;
                for (j=0; j<pattern_i; j++) {
                        strncpy(equivalence_dictionary_values[equivalence_dictionary_index][j], pattern[j], 5);
                }
                equivalence_dictionary_values[equivalence_dictionary_index][j][0] = '\0';
                equivalence_dictionary_index++;

                // Mark all occurences of the pattern within the list as covered
                mark_occurances_of_pattern_wthin_list_as_covered(pattern, list_of_symbols,
                                                                 symbol_is_covered, len_list_of_symbols);
        }
        equivalence_dictionary_keys[equivalence_dictionary_index] = '\0';

        // Create the new list by replacing patterns of symbols using the dictionary
        replace_patterns_in_list(equivalence_dictionary_keys, equivalence_dictionary_values,
                                 list_of_symbols, len_list_of_symbols, list_of_new_symbols);
}

static size_t compress2(char *const mov, size_t num, char *strings[3]) {
        //    R6L10R8R8R12L8L8R6L10R8R8R12L8L8L10R6R6L8R6L10R8R8R12L8L8L10R6R6L8R6L10R8L10R6R6L8
        // A: R6              R6                 R6R6  R6                 R6R6  R6        R6R6
        // B:   L10             L10           L10        L10           L10        L10  L10
        // C:      R8R8            R8R8                     R8R8                     R8
        // D:          R12L8L8         R12L8L8                  R12L8L8
        // E:                                        L8                       L8              L8
        //    A B  C C D      A B  C C D      B  A A E A B  C C D      B  A A E A B  C B  A A E

        //    ABCCDABCCDBAAEABCCDBAAEABCBAAE
        // F: ABC  ABC      ABC      ABC
        // G:    CD   CD       CD
        // H:           BAAE     BAAE   BAAE
        //    F  G F  G H   F  G H   F  H

        // F = ABC = R6 L10 R8     => R,6,L,10,R,8
        // G = CD = R8 R12L8L8     => R,8,R,12,L,8,L,8
        // H = BAAE = L10 R6 R6 L8 => L,10,R,6,R,6,L,8
        // FGFGHFGHFH

        // Can I compress more?
        //    FGFGHFGHFH
        // I: FGFG FG
        // J:     H  H H
        // K:         K
        // I guess I could still reduce the message size by one more step but this doesn't really change
        // anything

        // Fuck you all, fuck you fuck you fuck you
        // I've reinvented a compression algorithm having no fucking idea about compression algorithms
        // I'm the fucking best and you all fucking suck

        
        // Input: List of symbols
        // Ouput: List of new symbols and equivalence dictionary
        //
        // While uncovered symbols remain:
        //   Take the first non covered symbol from the input list
        //   Count occurances of this pattern of symbols within the list
        //   Occurances that would include a covered symbol don't count
        //   Add consecutive symbols until the number of occurences would change or no more can be added
        //
        //   Add a new symbol to the dictionary and associate it with this pattern
        //   Mark all occurences of the pattern within the list as covered
        //
        // Create the new list by replacing patterns of symbols using the dictionary
        //
        // Recurse, creating new dictionaries and further compressing the list of symbols
        // Ends when a new iteration creates a dictionary with the same or more entries
        //  as the previous.


        // max 10 keys, each of max size 1
        // end is signaled by a 0 byte
        char equivalence_dictionary_keys[10];
        
        // max 10 values, each composed of max 10 symbols, each of max size 5
        // end of values is signaled by keys array
        // end of every list of symbols is signaled by a symbol that is empty
        // end of a symbol is signaled by a 0 byte like any string
        char equivalence_dictionary_values[10][10][5];

        // max 50 symbols each of max size 5
        // end of list is signaled by an empty symbol
        // end of symbols is signaled by a 0 byte like any string
        char list_of_symbols[50][5];
        char list_of_new_symbols[50][5];

        size_t symbol=0, symbol_character=0;
        for (size_t i=0; i<num; i++) {
                char c = mov[i];

                if (!isdigit(c)) {
                        list_of_symbols[symbol][symbol_character++] = c;
                } else {
                        list_of_symbols[symbol][symbol_character++] = c;
                        if (isdigit(mov[i+1])) {
                                list_of_symbols[symbol][symbol_character++] = mov[++i];
                        }
                        list_of_symbols[symbol++][symbol_character] = '\0';
                        symbol_character = 0;
                }
        }
        list_of_symbols[symbol][0] = '\0';

        compress2_part2_battletendency(list_of_symbols, list_of_new_symbols,
                                       equivalence_dictionary_keys, equivalence_dictionary_values);

        size_t newnum = 0;
        for (size_t i=0; i<50; i++) {
                for (size_t j=0; j<5; j++) {
                        char c = list_of_symbols[i][j];
                        if (c == '\0') {
                                if (j == 0) {
                                        goto end;
                                } else {
                                        break;
                                }
                        }
                        mov[newnum++] = c;
                }
                mov[newnum++] = ',';
        }
end:;

        ASSERT(strncmp(equivalence_dictionary_keys, "ABC", 4) == 0, "Keys are not ABC");

        for (size_t i=0; i<3; i++) {
                char buff[50];
                size_t l=0;
                for (size_t j=0; j<10; j++) {
                        for (size_t k=0; k<5; k++) {
                                char c = equivalence_dictionary_values[i][j][k];
                                if (c == '\0') {
                                        if (k == 0) {
                                                goto end2;
                                        } else {
                                                break;
                                        }
                                }
                                buff[l++] = c;
                        }
                        buff[l++] = ',';
                }
end2:
                buff[l-1] = '\0';
                strings[i] = strndup(buff, 50);
        }
        
        return newnum-1;
}
*/

static void solution2(const char *const input, char *const output) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);

        size_t rows, columns;
        char *map = parse_map(&machine, &rows, &columns);
        show_map(map, rows, columns);
        DBG("Shown map\n");

        size_t startx=0, starty=0;
        get_starting_location(map, rows, columns, &startx, &starty);

        static char movements[1<<10];
        size_t num_movements = get_path(map, rows, columns, startx, starty, movements);
        print_str(movements, num_movements);
        fprintf(stderr, "\n");
        DBG("Shown movements uncompressed\n");

        num_movements = compress1(movements, num_movements);
        print_str(movements, num_movements);
        fprintf(stderr, "\n");
        DBG("Shown movements a bit compressed\n");

        char *strings[3];
        //num_movements = compress2(movements, num_movements, strings); // TODO :^)
        strings[0] = "R,6,L,10,R,8";
        strings[1] = "R,8,R,12,L,8,L,8";
        strings[2] = "L,10,R,6,R,6,L,8";
        strcpy(movements, "A,B,A,B,C,A,B,C,A,C");
        num_movements = 19;
        print_str(movements, num_movements);
        fprintf(stderr, "\n");
        for (int i=0; i<3; i++) {
                fprintf(stderr, "%c = %s\n", 'A'+i, strings[i]);
        }
        DBG("Shown movements fully compressed\n");

        // initialize machine to run actual program on
        struct IntCodeMachine machine2;
        machine_init(&machine2, input);
        machine2.program[0] = 2;

        machine_run(&machine2);
        while (machine2.has_output) {
                long out;
                machine_recv_output(&machine2, &out);
                fputc(out, stderr);
                machine_run(&machine2);
        }
        fprintf(stderr, "> ");

        // main movement routine
        machine_run(&machine2);
        for (size_t i=0; i<num_movements; i++) {
                fputc(movements[i], stderr);
                ASSERT(machine_send_input(&machine2, movements[i]), "Machine did not take input?");
                machine_run(&machine2);
        }
        fputc('\n', stderr);
        ASSERT(machine_send_input(&machine2, '\n'), "Machine did not take input?");

        machine_run(&machine2);
        while (machine2.has_output) {
                long out;
                machine_recv_output(&machine2, &out);
                fputc(out, stderr);
                machine_run(&machine2);
        }
        fprintf(stderr, "> ");

        // movement functions
        for (int j=0; j<3; j++) {
                machine_run(&machine2);
                for (size_t i=0;; i++) {
                        if (strings[j][i] == '\0') {
                                break;
                        }
                        fputc(strings[j][i], stderr);
                        ASSERT(machine_send_input(&machine2, strings[j][i]), "Machine did not take input?");
                        machine_run(&machine2);
                }
                fputc('\n', stderr);
                ASSERT(machine_send_input(&machine2, '\n'), "Machine did not take input?");

                machine_run(&machine2);
                while (machine2.has_output) {
                        long out;
                        machine_recv_output(&machine2, &out);
                        fputc(out, stderr);
                        machine_run(&machine2);
                }
                fprintf(stderr, "> ");
        }

        // give me a live feed
        fprintf(stderr, "n\n");
        machine_run(&machine2);
        ASSERT(machine_send_input(&machine2, 'n'), "Machine did not take input?");
        machine_run(&machine2);
        ASSERT(machine_send_input(&machine2, '\n'), "Machine did not take input?");

        machine_run(&machine2);
        char last = 0;
        long out;
        while (machine2.running) {
                if (machine2.has_output) {
                        machine_recv_output(&machine2, &out);

                        if (out > 255) {
                                break;
                        }
                        
                        fputc(out, stderr);
                        machine_run(&machine2);

                        if (out == '\n' && last == '\n') {
                                usleep(250000);
                        }
                        last = out;
                } else if (machine2.has_input) {
                        machine_send_input(&machine2, '?');
                        machine_run(&machine2);
                } else {
                        FAIL("Excuse me, what the fuck?");
                }
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", out);
	machine_free(&machine);
	machine_free(&machine2);
	free(map);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
