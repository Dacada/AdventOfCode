CC = gcc

GENERIC_CFLAGS = -Iaoclib -Ilib -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Werror -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=noreturn -Wsuggest-attribute=format -Wsuggest-attribute=cold -Wsuggest-attribute=malloc -std=c17
CFLAGS = $(GENERIC_CFLAGS) -Ofast -march=native
DEBUG_CFLAGS = $(GENERIC_CFLAGS) -Og -g -DDEBUG -fsanitize=address -fsanitize=undefined
GDB_CFLAGS = $(GENERIC_CFLAGS) -Og -g -DDEBUG

LDFLAGS = -Ofast
DEBUG_LDFLAGS = -Og -g -fsanitize=address -fsanitize=undefined
GDB_LDFLAGS = -Og -g

LDLIBS =


aoclib/aoclib_rel.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(CFLAGS) -c $< -o $@
aoclib/aoclib_dbg.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@
aoclib/aoclib_gdb.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(GDB_CFLAGS) -c $< -o $@


# Object files may be compiled using:
#
#  * Release flags, which disable sanitizers and enable optimizations,
#    asuming the program will not encounter unexpected cases (asserts
#    become no ops)
#
#  * DBG flags, which enable sanitizers and allow detecting various
#    errors and bugs easily during runtime thanks to assertions
#    (asserts become aborts)
#
#  * GDB flags, which disable sanitizers and allow debugging under
#    tools like GDB, keeping the debug mode for assertions (asserts
#    are still aborts)

%_rel.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
%_dbg.o: %.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@
%_gdb.o: %.c
	$(CC) $(GDB_CFLAGS) -c $< -o $@


2015/4_rel: LDLIBS = -lmd
2015/4_dbg: LDLIBS = -lmd
2015/4_gdb: LDLIBS = -lmd

2016/5_rel: LDLIBS = -lmd
2016/5_dbg: LDLIBS = -lmd -lncurses
2016/5_gdb: LDLIBS = -lmd -lncurses

2016/14_rel: LDLIBS = -lmd
2016/14_dbg: LDLIBS = -lmd
2016/14_gdb: LDLIBS = -lmd

2016/17_rel: LDLIBS = -lmd
2016/17_dbg: LDLIBS = -lmd
2016/17_gdb: LDLIBS = -lmd

2016/19_rel: LDLIBS = -lm
2016/19_dbg: LDLIBS = -lm
2016/19_gdb: LDLIBS = -lm


2019/13_rel: 2019/intcode.o
2019/13_dbg: LDLIBS = -lncurses
2019/13_dbg: 2019/intcode_dbg.o
2019/13_gdb: LDLIBS = -lncurses
2019/13_gdb: 2019/intcode_gdb.o

2019/15_rel: 2019/intcode.o
2019/15_dbg: 2019/intcode_dbg.o
2019/15_gdb: 2019/intcode_gdb.o

2019/17_rel: 2019/intcode.o
2019/17_dbg: 2019/intcode_dbg.o
2019/17_gdb: 2019/intcode_gdb.o

2019/19_rel: 2019/intcode.o
2019/19_dbg: 2019/intcode_dbg.o
2019/19_gdb: 2019/intcode_gdb.o

2019/21_rel: 2019/intcode.o
2019/21_dbg: 2019/intcode_dbg.o
2019/21_gdb: 2019/intcode_gdb.o

2019/23_rel: 2019/intcode.o
2019/23_dbg: 2019/intcode_dbg.o
2019/23_gdb: 2019/intcode_gdb.o

2019/25_rel: 2019/intcode.o
2019/25_dbg: 2019/intcode_dbg.o
2019/25_gdb: 2019/intcode_gdb.o


2020/20_rel: LDLIBS = -lm
2020/20_dbg: LDLIBS = -lm
2020/20_gdb: LDLIBS = -lm


2021/7_rel: LDLIBS = -lm
2021/7_dbg: LDLIBS = -lm
2021/7_gdb: LDLIBS = -lm


%_rel: %_rel.o aoclib/aoclib_rel.o
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@
%_dbg: %_dbg.o aoclib/aoclib_dbg.o
	$(CC) $(DEBUG_LDFLAGS) $(LDLIBS) $^ -o $@
%_gdb: %_gdb.o aoclib/aoclib_gdb.o
	$(CC) $(GDB_LDFLAGS) $(LDLIBS) $^ -o $@
