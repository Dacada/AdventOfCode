CC = gcc

CFLAGS = -Ilib -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Werror -std=c99 -Ofast
DEBUG_CFLAGS = -Ilib -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Werror -std=c99 -Og -g -DDEBUG -fsanitize=address -fsanitize=undefined
GDB_CFLAGS = -Ilib -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Werror -std=c99 -Og -g -DDEBUG

LDFLAGS = -std=c99 -Ofast
DEBUG_LDFLAGS = -std=c99 -Og -g -fsanitize=address -fsanitize=undefined
GDB_LDFLAGS = -std=c99 -Og -g

LDLIBS =

lib/jsmn/libjsmn.a:
	$(MAKE) -C lib/jsmn


aoclib/aoclib.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(CFLAGS) -c $< -o $@
aoclib/aoclib_dbg.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@
aoclib/aoclib_gdb.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(GDB_CFLAGS) -c $< -o $@

# Object files may be compiled using:
#  * Release flags, which disable sanitizers and enable optimizations, asuming the program will not encounter unexpected cases (asserts become no ops)
#  * DBG flags, which enable sanitizers and allow detecting various errors and bugs easily during runtime thanks to assertions (asserts become aborts)
#  * GDB flags, which disable sanitizers and allow debugging under tools like GDB, keeping the debug mode for assertions (asserts are still aborts)

2015/%.o: 2015/%.c
	$(CC) $(CFLAGS) -Iaoclib -c $< -o $@
2015/%_dbg.o: 2015/%.c
	$(CC) $(DEBUG_CFLAGS) -Iaoclib -c $< -o $@
2015/%_gdb.o: 2015/%.c
	$(CC) $(GDB_CFLAGS) -Iaoclib -c $< -o $@

2019/%.o: 2019/%.c
	$(CC) $(CFLAGS) -Iaoclib -c $< -o $@
2019/%_dbg.o: 2019/%.c
	$(CC) $(DEBUG_CFLAGS) -Iaoclib -c $< -o $@
2019/%_gdb.o: 2019/%.c
	$(CC) $(GDB_CFLAGS) -Iaoclib -c $< -o $@


2015/4: LDLIBS = -lbsd
2015/12: lib/jsmn/libjsmn.a
2015/%: 2015/%.o aoclib/aoclib.o
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

2019/13: 2019/intcode.o
2019/15: 2019/intcode.o
2019/17: 2019/intcode.o
2019/19: 2019/intcode.o
2019/21: 2019/intcode.o
2019/23: 2019/intcode.o
2019/25: 2019/intcode.o
2019/%: 2019/%.o aoclib/aoclib.o
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@


2015/4_dbg: LDLIBS = -lbsd
2015/12_dbg: lib/jsmn/libjsmn.a
2015/%_dbg: 2015/%_dbg.o aoclib/aoclib_dbg.o
	$(CC) $(DEBUG_LDFLAGS) $(LDLIBS) $^ -o $@

2019/13_dbg: LDLIBS = -lncurses
2019/13_dbg: 2019/intcode_dbg.o
2019/15_dbg: 2019/intcode_dbg.o
2019/17_dbg: 2019/intcode_dbg.o
2019/19_dbg: 2019/intcode_dbg.o
2019/21_dbg: 2019/intcode_dbg.o
2019/23_dbg: 2019/intcode_dbg.o
2019/25_dbg: 2019/intcode_dbg.o
2019/%_dbg: 2019/%_dbg.o aoclib/aoclib_dbg.o
	$(CC) $(DEBUG_LDFLAGS) $(LDLIBS) $^ -o $@


2015/4_gdb: LDLIBS = -lbsd
2015/12_gdb: lib/jsmn/libjsmn.a
2015/%_gdb: 2015/%_gdb.o aoclib/aoclib_gdb.o
	$(CC) $(GDB_LDFLAGS) $(LDLIBS) $^ -o $@

2019/13_gdb: LDLIBS = -lncurses
2019/13_gdb: 2019/intcode_gdb.o
2019/15_gdb: 2019/intcode_gdb.o
2019/17_gdb: 2019/intcode_gdb.o
2019/19_gdb: 2019/intcode_gdb.o
2019/21_gdb: 2019/intcode_gdb.o
2019/23_gdb: 2019/intcode_gdb.o
2019/25_gdb: 2019/intcode_gdb.o
2019/%_gdb: 2019/%_gdb.o aoclib/aoclib_gdb.o
	$(CC) $(GDB_LDFLAGS) $(LDLIBS) $^ -o $@
