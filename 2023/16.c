#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define IDX(array, x, y, width) ((array)[(x)+(y)*(width)])
//#define PRINT_STATE

enum direction {
	UP, LEFT, DOWN, RIGHT
};

struct beam {
	int x;
	int y;
	enum direction d;
};

static void beam_advance(struct beam *b) {
	switch (b->d) {
	case UP:
		b->y--; break;
	case LEFT:
		b->x--; break;
	case DOWN:
		b->y++; break;
	case RIGHT:
		b->x++; break;
	}
}

static int beam_cmp(const struct beam *b1, const struct beam *b2) {
	int cmp = b1->x - b2->x;
	if (cmp != 0) {
		return cmp;
	}
	cmp = b1->y - b2->y;
	if (cmp != 0) {
		return cmp;
	}
	return b1->d - b2->d;
}

struct state {
	int len;
	int cap;
	struct beam *beams;
};

static void state_init(struct state *state) {
	int len = 0;
	int cap = 1;
	struct beam *list = malloc(sizeof(*list)*cap);
	state->len = len;
	state->cap = cap;
	state->beams = list;
}

static void state_free(struct state *state) {
	free(state->beams);
}

static void state_print(const struct state *state) {
#ifdef PRINT_STATE
	fputc('[', stderr);
	for (int i=0; i<state->len; i++) {
		fputc('(', stderr);
		struct beam b = state->beams[i];
		fprintf(stderr, "%d, %d, ", b.x, b.y);
		switch (b.d) {
		case UP: fputs("UP", stderr); break;
		case LEFT: fputs("LEFT", stderr); break;
		case DOWN: fputs("DOWN", stderr); break;
		case RIGHT: fputs("RIGHT", stderr); break;
		}
		fputc(')', stderr);
		if (i != state->len-1) {
			fputs(", ", stderr);
		}
	}
	fputc(']', stderr);
	fputc('\n', stderr);
#else
	(void)state;
#endif
}

static void state_add_beam(struct state *state, struct beam beam) {
	if (state->len >= state->cap) {
		state->cap *= 2;
		state->beams = realloc(state->beams, sizeof(*state->beams)*state->cap);
	}
	
	state->beams[state->len++] = beam;
}

static void state_energize(const struct state *state, bool *energized, int width) {
	for (int i=0; i<state->len; i++) {
		IDX(energized, state->beams[i].x, state->beams[i].y, width) = true;
	}
}

struct bst {
	bool init;
	struct beam value;
	struct bst *left;
	struct bst *right;
};

static void bst_init(struct bst *bst) {
	bst->init = false;
}

static void bst_free(struct bst *bst) {
	if (bst == NULL || !bst->init) {
		return;
	}

	bst_free(bst->left);
	free(bst->left);
	bst_free(bst->right);
	free(bst->right);
}

static void bst_add(struct bst *bst, struct beam value) {
	if (!bst->init) {
		bst->init = true;
		bst->value = value;
		bst->left = NULL;
		bst->right = NULL;
		return;
	}

	int cmp = beam_cmp(&bst->value, &value);
	struct bst **node;
	if (cmp < 0) {
		node = &bst->left;
	} else if (cmp > 0) {
		node = &bst->right;
	} else {
		FAIL("attempt to add item already in bst");
	}

	if (*node == NULL) {
		*node = malloc(sizeof(**node));
		bst_init(*node);
	}
	bst_add(*node, value);
}

static bool bst_contains(const struct bst *bst, struct beam value) {
	if (bst == NULL || !bst->init) {
		return false;
	}

	int cmp = beam_cmp(&bst->value, &value);
	if (cmp < 0) {
		return bst_contains(bst->left, value);
	} else if (cmp > 0) {
		return bst_contains(bst->right, value);
	} else {
		return true;
	}
}

static char *parse_input(const char *input, int *width, int *height) {
	int len = 0;
	int cap = 16;
	char *list = malloc(sizeof(*list)*cap);

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
			list[len++] = c;
		}
		input++;
	}

	*width = w;
	*height = len / w;
	return list;
}

static void add_new_states(struct beam beam, char c, struct state *new_state, struct bst *seen_beams) {
	if (c == '/') {
		switch (beam.d) {
		case UP: beam.d = RIGHT; break;
		case LEFT: beam.d = DOWN; break;
		case DOWN: beam.d = LEFT; break;
		case RIGHT: beam.d = UP; break;
		}
	} else if (c == '\\') {
		switch (beam.d) {
		case UP: beam.d = LEFT; break;
		case LEFT: beam.d = UP; break;
		case DOWN: beam.d = RIGHT; break;
		case RIGHT: beam.d = DOWN; break;
		}
	} else if (c == '|') {
		if (beam.d == LEFT || beam.d == RIGHT) {
			struct beam beam2 = beam;
			beam.d = UP;
			beam2.d = DOWN;
			if (!bst_contains(seen_beams, beam2)) {
				bst_add(seen_beams, beam2);
				state_add_beam(new_state, beam2);
			}
		}
	} else if (c == '-') {
		if (beam.d == UP || beam.d == DOWN) {
			struct beam beam2 = beam;
			beam.d = LEFT;
			beam2.d = RIGHT;
			if (!bst_contains(seen_beams, beam2)) {
				bst_add(seen_beams, beam2);
				state_add_beam(new_state, beam2);
			}
		}
	}
	if (!bst_contains(seen_beams, beam)) {
		bst_add(seen_beams, beam);
		state_add_beam(new_state, beam);
	}
}

static void advance(const char *contraption, int width, int height, const struct state *state, struct state *new_state, struct bst *seen_beams) {
	for (int i=0; i<state->len; i++) {
		struct beam beam = state->beams[i];
		beam_advance(&beam);
		if (beam.x < 0 || beam.x >= width ||
		    beam.y < 0 || beam.y >= height) {
			continue;
		}
		char c = IDX(contraption, beam.x, beam.y, width);
		add_new_states(beam, c, new_state, seen_beams);
	}
}

static int energized(const char *contraption, int width, int height, int x, int y, int d) {
#ifdef PRINT_STATE
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			fputc(IDX(contraption, i, j, width), stderr);
		}
		fputc('\n', stderr);
	}
#endif

	struct bst seen_beams;
	bst_init(&seen_beams);

	struct state state;
	state_init(&state);
	{
		struct beam b;
		b.x = x;
		b.y = y;
		b.d = d;
		char c = IDX(contraption, x, y, width);;
		add_new_states(b, c, &state, &seen_beams);
	}

	bool *energized = malloc(sizeof(*energized)*width*height);
	for (int i=0; i<width*height; i++) {
		energized[i] = false;
	}
	state_energize(&state, energized, width);

	for (;;) {
		state_print(&state);
		
		struct state new_state;
		state_init(&new_state);
		advance(contraption, width, height, &state, &new_state, &seen_beams);
		if (new_state.len == 0) {
			state_free(&new_state);
			break;
		}

		state_energize(&new_state, energized, width);
		
		state_free(&state);
		state = new_state;
	}

	int res = 0;
	for (int i=0; i<width*height; i++) {
		if (energized[i]) {
			res++;
		}
	}

#ifdef PRINT_STATE
	fputc('\n', stderr);
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			if (IDX(energized, i, j, width)) {
				fputc('#', stderr);
			} else {
				fputc('.', stderr);
			}
		}
		fputc('\n', stderr);
	}
#endif
	
	free(energized);
	state_free(&state);
	bst_free(&seen_beams);
	return res;
}

static void solution1(const char *const input, char *const output) {
	int width, height;
	char *contraption = parse_input(input, &width, &height);
	int res = energized(contraption, width, height, 0, 0, RIGHT);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
	free(contraption);
}

static void solution2(const char *const input, char *const output) {
	int width, height;
	char *contraption = parse_input(input, &width, &height);

	int res = 0;
	for (int i=0; i<width; i++) {
		int n = energized(contraption, width, height, i, 0, DOWN);
		if (n > res) {
			res = n;
		}
		
		n = energized(contraption, width, height, i, height-1, UP);
		if (n > res) {
			res = n;
		}
	}
	for (int j=0; j<height; j++) {
		int n = energized(contraption, width, height, 0, j, RIGHT);
		if (n > res) {
			res = n;
		}
		
		n = energized(contraption, width, height, width-1, j, LEFT);
		if (n > res) {
			res = n;
		}
	}

        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
	free(contraption);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
