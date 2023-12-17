#include <aoclib.h>
#include <stdio.h>
#ifdef DEBUG
#include <unistd.h>
#endif

#define IDX(array, x, y, width) ((array)[(x) + (y)*(width)])

struct state {
	int x;
	int y;
  
	int dir_count;
	int prev_dir;

	int loss_count;

#ifdef DEBUG
	struct state *history;
	int histlen;
#endif
};

static void state_init(struct state *s, int x, int y) {
	s->x = x;
	s->y = y;
	
	s->dir_count = 0;
	s->prev_dir = -1;
	
	s->loss_count = 0;

#ifdef DEBUG
	s->history = NULL;
	s->histlen = 0;
#endif
}

static void state_free(struct state *s) {
#ifdef DEBUG
	free(s->history);
#else
	(void)s;
#endif
}

static void state_print(const struct state *s, const int *map, int width, int height) {
#ifdef DEBUG
	char *strmap = malloc(sizeof(*strmap)*width*height);
	for (int i=0; i<width*height; i++) {
		strmap[i] = map[i] + '0';
	}

	for (int i=0; i<s->histlen; i++) {
		char c;
		switch (s->history[i].prev_dir) {
		case 0: c = '^'; break;
		case 1: c = '<'; break;
		case 2: c = 'v'; break;
		case 3: c = '>'; break;
		default: c = '?'; break;
		}
		IDX(strmap, s->history[i].x, s->history[i].y, width) = c;
	}

	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			fputc(IDX(strmap, i, j, width), stderr);
		}
		fputc('\n', stderr);
	}
	fputc('\n', stderr);
	free(strmap);
#else
	(void)s; (void)map; (void)width; (void)height;
#endif
}

static bool state_init_next(struct state current, int i, struct state *next,
			    const int *map, int width, int height, bool ultra) {
	int maxdir = 3;
	if (ultra) {
		maxdir = 10;
	}
	
	if (current.prev_dir == 0 && i == 2) {
		return false;
	}
	if (current.prev_dir == 1 && i == 3) {
		return false;
	}
	if (current.prev_dir == 2 && i == 0) {
		return false;
	}
	if (current.prev_dir == 3 && i == 1) {
		return false;
	}

	if (current.prev_dir == i) {
		next->dir_count = current.dir_count + 1;
	} else {
		if (ultra) {
			next->dir_count = 3;
		} else {
			next->dir_count = 0;
		}
	}
	if (next->dir_count >= maxdir) {
		return false;
	}

	int inc = 1;
	if (ultra && current.prev_dir != i) {
		inc = 4;
	}

	next->x = current.x;
	next->y = current.y;
	next->prev_dir = i;
	next->loss_count = current.loss_count;

#ifdef DEBUG
	next->histlen = current.histlen;
	next->history = malloc(sizeof(*next->history)*(next->histlen+inc));
	for (int j=0; j<current.histlen; j++) {
		next->history[j] = current.history[j];
	}
	struct state prev = current;
#endif

	for (int j=0; j<inc; j++) {
#ifdef DEBUG
		next->history[next->histlen] = prev;
		next->histlen += 1;
#endif

		switch (i) {
		case 0: next->y--; break;
		case 1: next->x--; break;
		case 2: next->y++; break;
		case 3: next->x++; break;
		}
		if (next->x < 0 || next->x >= width) {
#ifdef DEBUG
			free(next->history);
#endif
			return false;
		}
		if (next->y < 0 || next->y >= height) {
#ifdef DEBUG
			free(next->history);
#endif
			return false;
		}
	
		next->loss_count += IDX(map, next->x, next->y, width);

#ifdef DEBUG
		prev = *next;
#endif
	}
	
	return true;
}

static int state_value(const struct state *s) {
	return s->loss_count;
}

struct heap_element {
	struct state element;
	int value;
};

struct heap {
	int cap;
	int len;
	struct heap_element *list;
};

static void heap_init(struct heap *heap) {
	heap->cap = 16;
	heap->len = 0;
	heap->list = malloc(sizeof(*heap->list)*heap->cap);
}

static void heap_free(struct heap *heap) {
	for (int i=0; i<heap->len; i++) {
		state_free(&heap->list[i].element);
	}
	free(heap->list);
}

static void heap_add(struct heap *heap, struct state element) {
	if (heap->len >= heap->cap) {
		heap->cap *= 2;
		heap->list = realloc(heap->list, sizeof(*heap->list)*heap->cap);
	}
	struct heap_element e;
	e.element = element;
	e.value = state_value(&element);
	heap->list[heap->len++] = e;
	
	int i = heap->len - 1;
	int p = (i - 1) / 2;
	while (i > 0 && heap->list[p].value > heap->list[i].value) {
		struct heap_element tmp = heap->list[i];
		heap->list[i] = heap->list[p];
		heap->list[p] = tmp;

		i = p;
		p = (i - 1) / 2;
	}
}

static struct state heap_pop(struct heap *heap) {
	struct state element = heap->list[0].element;
	heap->list[0] = heap->list[--heap->len];

	int i = 0;
	for (;;) {
		int c1 = 2 * i + 1;
		int c2 = 2 * i + 2;
		
		int s = i;
		if (c1 < heap->len && heap->list[c1].value < heap->list[s].value) {
			s = c1;
		}
		if (c2 < heap->len && heap->list[c2].value < heap->list[s].value) {
			s = c2;
		}

		if (s != i) {
			struct heap_element tmp = heap->list[i];
			heap->list[i] = heap->list[s];
			heap->list[s] = tmp;
			i = s;
		} else {
			break;
		}
	}

	return element;
}

static int *parse_input(const char *input, int *width, int *height) {
	int len = 0;
	int cap = 16;
	int *list = malloc(sizeof(*list)*cap);

	int w = 0;
	while (*input != '\0') {
		if (len >= cap) {
			cap *= 2;
			list = realloc(list, sizeof(*list)*cap);
		}
		char c = *input;
		if (c == '\n') {
			if (w == 0) {
				w = len;
			}
		} else {
			list[len++] = c - '0';
		}
		input++;
	}

	*width = w;
	*height = len / w;
	return list;
}

static int search(const int *map, int width, int height, bool ultra) {
	int ndirs = 4;
	int nconsecdirs = 3;
	if (ultra) {
		nconsecdirs = 10;
	}
	
	bool visited[width][height][ndirs][nconsecdirs];
	for (int i=0; i<width; i++) {
		for (int j=0; j<height; j++) {
			for (int k=0; k<ndirs; k++) {
				for (int l=0; l<nconsecdirs; l++) {
					visited[i][j][k][l] = false;
				}
			}
		}
	}

	struct heap heap;
	heap_init(&heap);

	struct state current;
	state_init(&current, 0, 0);

	heap_add(&heap, current);

	for (;;) {
		current = heap_pop(&heap);
		
		//DBG("value=%d", state_value(&current));
		//state_print(&current, map, width, height);
		//sleep(1);

		if (current.prev_dir >= 0) {
			if (visited[current.x][current.y][current.prev_dir][current.dir_count]) {
				state_free(&current);
				continue;
			}
			visited[current.x][current.y][current.prev_dir][current.dir_count] = true;
		}
		
		if (current.x == width - 1 && current.y == height - 1) {
			break;
		}
		
		for (int i=0; i<ndirs; i++) {
			struct state next;
			if (state_init_next(current, i, &next, map, width, height, ultra)) {
				heap_add(&heap, next);
			}
		}
		state_free(&current);
	}

	state_print(&current, map, width, height);

	state_free(&current);
	heap_free(&heap);
	
	return current.loss_count;
}

static void solution1(const char *const input, char *const output) {
	int width, height;
	int *map = parse_input(input, &width, &height);

#ifdef DEBUG
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			fputc(IDX(map, i, j, width)+'0', stderr);
		}
		fputc('\n', stderr);
	}
	fputc('\n', stderr);
#endif

	int res = search(map, width, height, false);

        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
	free(map);
}

static void solution2(const char *const input, char *const output) {
	int width, height;
	int *map = parse_input(input, &width, &height);

	int res = search(map, width, height, true);

        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
	free(map);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
