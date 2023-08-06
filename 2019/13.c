#ifdef DEBUG
#define _DEFAULT_SOURCE
#endif

#include "intcode.h"
#include <aoclib.h>
#include <stdio.h>

#ifdef DEBUG
#include <unistd.h>
#include <ncurses.h>
#endif

enum tile {
        EMPTY,
        WALL,
        BLOCK,
        HPADDLE,
        BALL
};

static void solution1(const char *const input, char *const output) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);

        int count = 0;
        machine_run(&machine);
        while (machine.running) {
                long x, y, t;
                
                ASSERT(machine_recv_output(&machine, &x), "expected machine output but got nuthin'");
                
                machine_run(&machine);
                ASSERT(machine_recv_output(&machine, &y), "expected machine output but got nuthin'");
                
                machine_run(&machine);
                ASSERT(machine_recv_output(&machine, &t), "expected machine output but got nuthin'");

                if (t == BLOCK) {
                        count++;
                }
                
                machine_run(&machine);
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
	machine_free(&machine);
}

#ifdef DEBUG
static void endwin_wrapper(void) {
        endwin();
}
#endif

static void solution2(const char *const input, char *const output) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);
        machine.program[0] = 2;
        
        #ifdef DEBUG
        initscr();
        atexit(endwin_wrapper);
        noecho();
        raw();
        nodelay(stdscr, true);
        keypad(stdscr, true);
        curs_set(0);
        #endif

        bool blocks[50][50];
        for(int j=0;j<50;j++){
                for(int i=0;i<50;i++){
                        blocks[j][i] = false;
                }
        }
        int count = 0;
        bool firstblock = false;

        long score = -1;
        int ballx=-1, paddlex=-1;
        long joystick = 0;
        machine_run(&machine);
        while (machine.running) {
                long x, y, t;
                
                ASSERT(machine_recv_output(&machine, &x), "expected machine output but got nuthin'");
                
                machine_run(&machine);
                ASSERT(machine_recv_output(&machine, &y), "expected machine output but got nuthin'");
                
                machine_run(&machine);
                ASSERT(machine_recv_output(&machine, &t), "expected machine output but got nuthin'");

                if (x == -1 && y == 0) {
                        score = t;
                        
                        #ifdef DEBUG
                        mvprintw(1, 50, "Score: %ld", score);
                        #endif

			if (count <= 0 && firstblock) {
			  break;
			}
                }

                enum tile tile = t;

                if (tile == BALL) {
                        ballx = x;
                        
                        #ifdef DEBUG
                        mvprintw(3, 50, "Ball X: %d      ", ballx);
                        #endif
                } else if (tile == HPADDLE) {
                        paddlex = x;
                        
                        #ifdef DEBUG
                        mvprintw(4, 50, "Paddle X: %d    ", paddlex);
                        #endif
                }

                if (tile == BLOCK) {
                        firstblock = true;
                        if (x > -1 && !blocks[y][x]) {
                                blocks[y][x] = true;
                                count++;
                        
                                #ifdef DEBUG
                                mvprintw(6, 50, "Blocks: %d    ", count);
                                #endif
                        }
                } else {
                        if (x > -1 && blocks[y][x]) {
                                blocks[y][x] = false;
                                count--;
                        
                                #ifdef DEBUG
                                mvprintw(6, 50, "Blocks: %d    ", count);
                                #endif
                        }
                }

                if (ballx > 0 && paddlex > 0) {
                        int diff = ballx - paddlex;
                        
                        if (diff == 0) {
                                joystick = 0;
                        } else {
                                if (diff < 0) {
                                        joystick = -1;
                                } else {
                                        joystick = 1;
                                }
                        }
                }
                        
#ifdef DEBUG
                char c;
                switch (tile) {
                case EMPTY:
                        c = ' '; break;
                case WALL:
                        c = '#'; break;
                case BLOCK:
                        c = '$'; break;
                case HPADDLE:
                        c = '_'; break;
                case BALL:
                        c = '*'; break;
                default:
                        c = '?'; break;
                }
                mvaddch(y, x, c);
                refresh();

                if (c == '*' || c == '_') {
                        usleep(20000);
                }
#endif

                machine_run(&machine);
                if (!machine.has_output && !machine.has_input) {
                        machine_send_input(&machine, joystick);
                        machine_run(&machine);
                        joystick = 0;
                }
        }

        #ifdef DEBUG
        endwin();
        #endif
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", score);
	machine_free(&machine);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
