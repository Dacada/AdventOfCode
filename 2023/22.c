#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define IDX_RAW(array, x, y, z, width, depth) ((array)[(x) + (y)*(depth) + (z)*(width)*(depth)])
#define IDX(array, point, width, depth) IDX_RAW(array, (point).x, (point).y, (point).z, width, depth)

struct point {
	int x;
	int y;
	int z;
};

struct brick {
	struct point location;
	int from;
	int to;
};

static struct point brick_point(struct brick b, int n) {
	int x = b.location.x;
	int y = b.location.y;
	int z = b.location.z;
	
	if (x < 0) {
		x = n;
	} else if (y < 0) {
		y = n;
	} else if (z < 0) {
		z = n;
	}

	return (struct point){x, y, z};
}

struct state {
	int *space;
	int width, depth, height;
	struct brick *bricks;
	int nbricks;
};

static void state_duplicate(struct state *dst, const struct state *src) {
	dst->width = src->width;
	dst->depth = src->depth;
	dst->height = src->height;
	dst->nbricks = src->nbricks;

	size_t s = sizeof(*dst->space)*dst->width*dst->depth*dst->height;
	dst->space = malloc(s);
	memcpy(dst->space, src->space, s);

	s = sizeof(*dst->bricks)*dst->nbricks;
	dst->bricks = malloc(s);
	memcpy(dst->bricks, src->bricks, s);
}

static void state_free(struct state *state) {
	free(state->space);
	free(state->bricks);
}

static void place_bricks(struct state *state) {
	int width = 0;
	int depth = 0;
	int height = 0;
	for (int i=0; i<state->nbricks; i++) {
		struct brick b = state->bricks[i];
		struct point p = brick_point(b, b.to);
		if (p.x > width) {
			width = p.x;
		}
		if (p.y > depth) {
			depth = p.y;
		}
		if (p.z > height) {
			height = p.z;
		}
	}
	width++;
	depth++;
	height++;

	int *space = malloc(sizeof(*space)*width*depth*height);
	for (int i=0; i<width*depth*height; i++) {
		space[i] = -1;
	}

	for (int i=0; i<state->nbricks; i++) {
		struct brick b = state->bricks[i];
		for (int j=b.from; j<=b.to; j++) {
			struct point p = brick_point(b, j);
			IDX(space, p, width, depth) = i;
		}
	}

	state->space = space;
	state->width = width;
	state->depth = depth;
	state->height = height;
}

static int parse_int(const char **input) {
	ASSERT(isdigit(**input), "parse error");

	int n=0;
	while (isdigit(**input)) {
		n *= 10;
		n += **input - '0';
		*input += 1;
	}

	return n;
}

static struct point parse_point(const char **input) {
	struct point p;
	
	p.x = parse_int(input);
	ASSERT(**input == ',', "parse error");
	*input += 1;
	p.y = parse_int(input);
	ASSERT(**input == ',', "parse error");
	*input += 1;
	p.z = parse_int(input);

	return p;
}

static struct brick parse_line(const char **input) {
	struct point p1 = parse_point(input);
	ASSERT(**input == '~', "parse error");
	*input += 1;
	struct point p2 = parse_point(input);
	if (**input == '\n') {
		*input += 1;
	}

	struct brick b;

	struct point loc = {-1, -1, -1};
	int count = 0;
	for (int i=0; i<3; i++) {
		int x1 = *(&p1.x + i);
		int x2 = *(&p2.x + i);
		if (x1 == x2) {
			*(&loc.x + i) = x1;
		} else {
			count++;
			if (x1 < x2) {
				b.from = x1;
				b.to = x2;
			} else {
				b.from = x2;
				b.to = x1;
			}
		}
	}

	if (count == 0) {
		b.from = b.to = loc.x;
		loc.x = -1;
	} else if (count > 1) {
		FAIL("brick is not a line");
	}

	b.location = loc;

	return b;
}

static void parse_input(const char *input, struct state *state) {
	int len = 0;
	int cap = 16;
	struct brick *list = malloc(sizeof(*list)*cap);
	
	while (*input != '\0') {
		if (len >= cap) {
			cap *= 2;
			list = realloc(list, sizeof(*list)*cap);
		}

		list[len++] = parse_line(&input);
	}

	state->bricks = list;
	state->nbricks = len;

	place_bricks(state);
}

static int brick_find_fall_height(const struct state *state, int brick_idx, int current_z) {
	struct brick b = state->bricks[brick_idx];

	int from, to;
	if (b.location.z < 0) {
		// columns get a special treatment
		from = to = b.from;
		if (from == 1) {
			// this column is already touching the ground
			return 0;
		}
	} else {
		from = b.from;
		to = b.to;
	}

	int h;
	for (h=1; h<current_z; h++) {
		bool found_collision = false;
		for (int l=from; l<=to; l++) {
			struct point p = brick_point(b, l);
			p.z -= h;
			if (IDX(state->space, p, state->width, state->depth) >= 0) {
				found_collision = true;
				break;
			}
		}
		if (found_collision) {
			break;
		}
	}
	
	// we found a collision at this negative height so we go 1 above
	return h - 1;
}

static void brick_do_fall(struct state *state, int brick_idx, int height) {
	struct brick brick = state->bricks[brick_idx];
	for (int n=brick.from; n<=brick.to; n++) {
		struct point current = brick_point(brick, n);
		struct point next = current;
		next.z -= height;

		// this still works for columns
		IDX(state->space, current, state->width, state->depth) = -1;
		IDX(state->space, next, state->width, state->depth) = brick_idx;
	}

	struct brick *ptr = &state->bricks[brick_idx];
	if (ptr->location.z < 0) {
		ptr->from -= height;
		ptr->to -= height;
	} else {
		ptr->location.z -= height;
	}
}

static int bricks_fall(struct state *state, int start) {
	int fell_count = 0;
	for (int z=start; z<state->height; z++) {
		for (int y=0; y<state->depth; y++) {
			for (int x=0; x<state->width; x++) {
				int i = IDX_RAW(state->space, x, y, z, state->width, state->depth);
				if (i >= 0) {
					// found a brick potentially floating above ground
					int h = brick_find_fall_height(state, i, z);
					if (h > 0) {
						fell_count++;
						brick_do_fall(state, i, h);
					}
				}
			}
		}
	}
	return fell_count;
}

static int count_bricks(const struct state *state, bool safe) {
	int count = 0;
	for (int i=0; i<state->nbricks; i++) {
		struct state new_state;
		state_duplicate(&new_state, state);
		struct brick b = state->bricks[i];
		for (int n=b.from; n<=b.to; n++) {
			struct point p = brick_point(b, n);
			IDX(new_state.space, p, state->width, state->depth) = -1;
		}

		int start = b.location.z;
		if (start < 0) {
			start = b.to;
		}
		start++;
		int fell = bricks_fall(&new_state, start);
		
		if (safe) {
			if (fell == 0) {
				count++;
			}
		} else {
			count += fell;
		}
		
		state_free(&new_state);
	}
	return count;
}

static int count_safe_bricks(const struct state *state) {
	return count_bricks(state, true);
}

static void solution1(const char *const input, char *const output) {
	struct state state;
	parse_input(input, &state);

	bricks_fall(&state, 2);
	
	snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count_safe_bricks(&state));
	state_free(&state);
}

static int count_unsafe_bricks(const struct state *state) {
	return count_bricks(state, false);
}

static void solution2(const char *const input, char *const output) {
	struct state state;
	parse_input(input, &state);

	bricks_fall(&state, 2);
	
	snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count_unsafe_bricks(&state));
	state_free(&state);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
