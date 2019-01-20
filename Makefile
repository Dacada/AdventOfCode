CC = gcc
CFLAGS = -Wall -Wextra -Wformat -Wshadow -Wpointer-arith -Wcast-qual -Wmissing-prototypes -std=c99 -Ofast
LDFLAGS = -Wall -Wextra -std=c99 -Ofast
LDLIBS =

aoclib/aoclib.o: aoclib/aoclib.c
	$(CC) $(CFLAGS) -c $^ -o $@

2015/%.o: 2015/%.c
	$(CC) $(CFLAGS) -Iaoclib -c $^ -o $@

2015/4: LDLIBS = -lbsd
2015/%: 2015/%.o aoclib/aoclib.o
	$(CC) $(LDFLAGS) $(LDLIBS) $^ -o $@
