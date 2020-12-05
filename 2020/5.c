#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static int boardingpass_to_id(const char *const text) {
        unsigned id = 0;
        for (int i=0; i<7; i++) {
                if (text[i] == 'F') {
                        id = (id << 1) + 0;
                } else if (text[i] == 'B') {
                        id = (id << 1) + 1;
                } else {
                        FAIL("invalid input");
                }
        }
        for (int i=7; i<10; i++) {
                if (text[i] == 'L') {
                        id = (id << 1) + 0;
                } else if (text[i] == 'R') {
                        id = (id << 1) + 1;
                } else {
                        FAIL("invalid input");
                }
        }
        return id;
}

static void solution1(const char *const input, char *const output) {
        int maxid = 0;
        for (size_t i=0;; i++) {
                if (input[i] == '\0') {
                        break;
                }
                int id = boardingpass_to_id(input+i);
                i += 10;
                if (id > maxid) {
                        maxid = id;
                }
        }
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", maxid);
}

static void solution2(const char *const input, char *const output) {
        const int nseats = 1<<10;
        
        bool seats[nseats];
        memset(seats, 0, sizeof(seats));
        for (size_t i=0;; i++) {
                if (input[i] == '\0') {
                        break;
                }
                int id = boardingpass_to_id(input+i);
                i += 10;
                seats[id] = true;
        }
        
        bool start = true;
        int myseat;
        for (myseat=0; myseat<nseats; myseat++) {
                if (start) {
                        if (seats[myseat]) {
                                start = false;
                        } else {
                        }
                } else {
                        if (seats[myseat]) {
                        } else {
                                break;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", myseat);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
