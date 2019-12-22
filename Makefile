CC = gcc

CFLAGS = -Ilib -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Werror -std=c99 -Ofast
DEBUG_CFLAGS = -Ilib -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Werror -std=c99 -Og -g -DDEBUG -fsanitize=address -fsanitize=undefined

LDFLAGS = -std=c99 -Ofast
DEBUG_LDFLAGS = -std=c99 -Og -g -fsanitize=address -fsanitize=undefined

LDLIBS =

lib/jsmn/libjsmn.a:
	$(MAKE) -C lib/jsmn


aoclib/aoclib.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(CFLAGS) -c $< -o $@
aoclib/aoclib_dbg.o: aoclib/aoclib.c aoclib/aoclib.h
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@


2015/%.o: 2015/%.c
	$(CC) $(CFLAGS) -Iaoclib -c $< -o $@
2015/%_dbg.o: 2015/%.c
	$(CC) $(DEBUG_CFLAGS) -Iaoclib -c $< -o $@

2019/%.o: 2019/%.c
	$(CC) $(CFLAGS) -Iaoclib -c $< -o $@
2019/%_dbg.o: 2019/%.c
	$(CC) $(DEBUG_CFLAGS) -Iaoclib -c $< -o $@


2015/4: LDLIBS = -lbsd
2015/12: lib/jsmn/libjsmn.a
2015/%: 2015/%.o aoclib/aoclib.o
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@

2019/13: 2019/intcode.o
2019/15: 2019/intcode.o
2019/17: 2019/intcode.o
2019/19: 2019/intcode.o
2019/21: 2019/intcode.o
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
2019/%_dbg: 2019/%_dbg.o aoclib/aoclib_dbg.o
	$(CC) $(DEBUG_LDFLAGS) $(LDLIBS) $^ -o $@
