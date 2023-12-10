#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define IDX_RAW(array, x, y, width) ((array)[(x) + (y) * (width)])
#define IDX(array, point, width) IDX_RAW(array, (point).x, (point).y, width)

enum pipe {
	VERTICAL = '|',
	HORIZONTAL = '-',
	NE_BEND = 'L',
	NW_BEND = 'J',
	SW_BEND = '7',
	SE_BEND = 'F',
	GROUND = '.',
	START = 'S',
};

const char *dirname[] = {"NORTH", "SOUTH", "EAST", "WEST", "NORTHEAST", "NORTHWEST", "SOUTHEAST", "SOUTHWEST"};
enum direction {
	NORTH, SOUTH, EAST, WEST,
	NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST,
};

struct tile_info {
	bool is_pipe;
	bool is_outside;
	bool is_inside;
	bool visiting;
	enum direction out;
};

struct point {
	int x;
	int y;
};

__attribute__((pure))
static enum direction dir_tick_cw(enum direction d) {
	switch (d) {
	case NORTH: return NORTHEAST;
	case NORTHEAST: return EAST;
	case EAST: return SOUTHEAST;
	case SOUTHEAST: return SOUTH;
	case SOUTH: return SOUTHWEST;
	case SOUTHWEST: return WEST;
	case WEST: return NORTHWEST;
	case NORTHWEST: return NORTH;
	}
	FAIL("invalid direction");
}

__attribute__((pure))
static enum direction dir_tick_ccw(enum direction d) {
	switch (d) {
	case NORTH: return NORTHWEST;
	case NORTHWEST: return WEST;
	case WEST: return SOUTHWEST;
	case SOUTHWEST: return SOUTH;
	case SOUTH: return SOUTHEAST;
	case SOUTHEAST: return EAST;
	case EAST: return NORTHEAST;
	case NORTHEAST: return NORTH;
	}
	FAIL("invalid direction");
}

static bool point_eq(struct point p1, struct point p2) {
	return p1.x == p2.x && p1.y == p2.y;
}

__attribute__((pure))
static struct point point_advance(struct point p, enum direction d) {
	struct point n = p;
	switch (d) {
	case NORTH:
		n.y -= 1;
		break;
	case SOUTH:
		n.y += 1;
		break;
	case EAST:
		n.x += 1;
		break;
	case WEST:
		n.x -= 1;
		break;
	default:
		FAIL("invalid direction %d", d);
	}
	return n;
}

__attribute__((pure))
static enum direction opposite(enum direction dir) {
	switch (dir) {
	case NORTH: return SOUTH;
	case SOUTH: return NORTH;
	case EAST: return WEST;
	case WEST: return EAST;
	default:
		FAIL("invalid direction %d", dir);
	}
}

__attribute__((pure))
static bool is_direction_valid_to_connect(enum pipe p, enum direction dir) {
	switch (p) {
	case VERTICAL: return dir == NORTH || dir == SOUTH;
	case HORIZONTAL: return dir == EAST || dir == WEST;
	case NE_BEND: return dir == NORTH || dir == EAST;
	case NW_BEND: return dir == NORTH || dir == WEST;
	case SW_BEND: return dir == SOUTH || dir == WEST;
	case SE_BEND: return dir == SOUTH || dir == EAST;
	case GROUND: return false;
	case START: return true;
	}
	FAIL("invalid pipe %c", p);
}

static void pipe_next(enum pipe *maze, struct point current, struct point *next1, struct point *next2, int width, int height) {
	bool next1_set = false;
	enum pipe current_pipe = IDX(maze, current, width);
	for (enum direction dir=0; dir<4; dir++) {
		if (!is_direction_valid_to_connect(current_pipe, dir)) {
			continue;
		}
		struct point next = point_advance(current, dir);
		if (next.x < 0 || next.x >= width || next.y < 0 || next.y >= height) {
			continue;
		}
		enum pipe next_pipe = IDX(maze, next, width);
		if (!is_direction_valid_to_connect(next_pipe, opposite(dir))) {
			continue;
		}
		if (!next1_set) {
			*next1 = next;
			next1_set = true;
		} else {
			*next2 = next;
			return;
		}
	}
	FAIL("could not find two valid connections for pipe");
}

static enum pipe *parse_input(const char *input, int *width, int *height, struct point *start) {
	DBG("\n%s", input);
	for (int i=0;; i++) {
		if (input[i] == '\n') {
			*width = i;
			break;
		}
	}
	for (int i=0;; i++) {
		if (input[i] == 0 || (input[i] == '\n' && input[i+1] == '\0')) {
			*height = i / *width;
			break;
		}
	}
	
	enum pipe *res = malloc(sizeof(*res)**width**height);

	int j = 0;
	for (int i=0; input[i] != '\0'; i++) {
		if (input[i] == '\n') {
			continue;
		}
		res[j] = input[i];
		if (input[i] == 'S') {
			start->x = j % *width;
			start->y = j / *width;
		}
		j++;
	}

	return res;
}

__attribute__((pure))
static enum pipe brute_force_start_sign(const enum pipe *maze, struct point start, int width, int height) {
	bool dirs[4];
	int n = 0;
	for (enum direction dir=0; dir<4; dir++) {
		struct point p = point_advance(start, dir);
		if (p.x < 0 || p.x >= width || p.y < 0 || p.y >= height) {
			dirs[dir] = false;
			continue;
		}
		dirs[dir] = is_direction_valid_to_connect(IDX(maze, p, width), opposite(dir));
		n += dirs[dir];
	}
	if (n != 2) {
		FAIL("impossible to get start sign");
	}
	if (dirs[NORTH] && dirs[EAST]) {
		return NE_BEND;
	}
	if (dirs[NORTH] && dirs[SOUTH]) {
		return VERTICAL;
	}
	if (dirs[NORTH] && dirs[WEST]) {
		return NW_BEND;
	}
	if (dirs[EAST] && dirs[SOUTH]) {
		return SE_BEND;
	}
	if (dirs[EAST] && dirs[WEST]) {
		return HORIZONTAL;
	}
	if (dirs[SOUTH] && dirs[WEST]) {
		return SW_BEND;
	}
	FAIL("impossible to get start sign");
}

static int find_loop(enum pipe *maze, struct point start, int width, int height, struct tile_info *maze_info) {
	int res = 1;
	
	struct point heads[2];
	DBG("start -> %d,%d '%c'", start.x, start.y, IDX(maze, start, width));
	pipe_next(maze, start, &heads[0], &heads[1], width, height);

	if (maze_info != NULL) {
		IDX(maze_info, start, width).is_pipe = true;
	}

	IDX(maze, start, width) = brute_force_start_sign(maze, start, width, height);
	
	struct point prev[2];
	prev[0] = start;
	prev[1] = start;
	
	while (!point_eq(heads[0], heads[1])) {
		for (int i=0; i<2; i++) {
			DBG("heads[%d] -> %d,%d '%c'", i, heads[i].x, heads[i].y, maze[heads[i].x + heads[i].y * width]);
			if (maze_info != NULL) {
				IDX(maze_info, heads[i], width).is_pipe = true;
			}
	
			struct point next1, next2;
			pipe_next(maze, heads[i], &next1, &next2, width, height);
			struct point tmp = heads[i];
			if (point_eq(next1, prev[i])) {
				heads[i] = next2;
			} else {
				heads[i] = next1;
			}
			prev[i] = tmp;
		}
		res++;
	}
	DBG("END");
	for (int i=0; i<2; i++) {
		DBG("heads[%d] -> %d,%d '%c'", i, heads[i].x, heads[i].y, IDX(maze, heads[i], width));
	}
	if (maze_info != NULL) {
		IDX(maze_info, heads[0], width).is_pipe = true;
	}

	return res;
}

static void solution1(const char *const input, char *const output) {
	int width, height;
	struct point start;
	enum pipe *maze = parse_input(input, &width, &height, &start);

#ifdef DEBUG
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			fprintf(stderr, "%c", IDX_RAW(maze, i, j, width));
		}
		fprintf(stderr, "\n");
	}
#endif

	int res = find_loop(maze, start, width, height, NULL);

	free(maze);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

static bool is_inside(const enum pipe *maze, struct tile_info *maze_info, struct point p, int width, int height) {
	struct tile_info *tile = &IDX(maze_info, p, width);
	if (tile->is_pipe) {
		DBG("%d,%d is not inside, it's a pipe", p.x, p.y);
		return false;
	}
	
	if (tile->is_outside) {
		DBG("%d,%d already found to be ouside", p.x, p.y);
		return false;
	}
	if (tile->is_inside) {
		DBG("%d,%d already found to be inside", p.x, p.y);
		return true;
	}

	struct point n = p;
	for (int i=0; i<2; i++) {
		if (i == 0) {
			n.x += 1;
		} else {
			n.y += 1;
		}
		
		if (n.x >= width || n.y >= height) {
			tile->is_outside = true;
			tile->is_inside = false;
			DBG("%d,%d has a neighbor out of bounds, it's outside", p.x, p.y);
			return false;
		}
		
		struct tile_info *next_tile = &IDX(maze_info, n, width);
		if (next_tile->is_pipe) {
			bool cond;
			if (i == 0) {
				cond = next_tile->out == WEST || next_tile->out == NORTHWEST || next_tile->out == SOUTHWEST;
			} else {
				cond = next_tile->out == NORTH || next_tile->out == NORTHWEST || next_tile->out == NORTHEAST;
			}
			if (cond) {
				tile->is_outside = true;
				tile->is_inside = false;
				DBG("%d,%d is next to a pipeline pointing to it, it's outside", p.x, p.y);
				return false;
			} else {
				tile->is_outside = false;
				tile->is_inside = true;
				DBG("%d,%d is next to a pipeline not pointing to it, it's inside", p.x, p.y);
				return true;
			}
		}
		
		if (!next_tile->visiting) {
			tile->visiting = true;
			tile->is_inside = is_inside(maze, maze_info, n, width, height);
			tile->visiting = false;
			tile->is_outside = !tile->is_inside;
			DBG("%d,%d is next to another tile that is %s", p.x, p.y, tile->is_inside?"inside":"outside");
			return tile->is_inside;
		}
		if (i == 0) {
			n.x -= 1;
		} else {
			n.y -= 1;
		}
	}
	FAIL("could not find if tile is in");
}

__attribute__((pure))
static enum direction follow_out_direction(enum pipe curr, enum pipe prev, enum direction prev_dir) {
	if (curr == prev) {
		return prev_dir;
	}

	switch (prev) {
	case VERTICAL:
		switch (curr) {
		case NE_BEND:
		case SW_BEND:
			return dir_tick_ccw(prev_dir);
		case NW_BEND:
		case SE_BEND:
			return dir_tick_cw(prev_dir);
		default:
			FAIL("impossible combination");
		}
	case HORIZONTAL:
		switch (curr) {
		case NE_BEND:
		case SW_BEND:
			return dir_tick_cw(prev_dir);
		case NW_BEND:
		case SE_BEND:
			return dir_tick_ccw(prev_dir);
		default:
			FAIL("impossible combination");
		}
	case NE_BEND:
		switch (curr) {
		case VERTICAL:
			return dir_tick_cw(prev_dir);
		case HORIZONTAL:
			return dir_tick_ccw(prev_dir);
		case SW_BEND:
			return prev_dir;
		case NW_BEND:
			return dir_tick_ccw(dir_tick_ccw(prev_dir));
		case SE_BEND:
			return dir_tick_cw(dir_tick_cw(prev_dir));
		default:
			FAIL("impossible combination");
		}
	case SW_BEND:
		switch (curr) {
		case VERTICAL:
			return dir_tick_cw(prev_dir);
		case HORIZONTAL:
			return dir_tick_ccw(prev_dir);
		case NE_BEND:
			return prev_dir;
		case SE_BEND:
			return dir_tick_ccw(dir_tick_ccw(prev_dir));
		case NW_BEND:
			return dir_tick_cw(dir_tick_cw(prev_dir));
		default:
			FAIL("impossible combination");
		}
	case NW_BEND:
		switch (curr) {
		case VERTICAL:
			return dir_tick_ccw(prev_dir);
		case HORIZONTAL:
			return dir_tick_cw(prev_dir);
		case SE_BEND:
			return prev_dir;
		case NE_BEND:
			return dir_tick_cw(dir_tick_cw(prev_dir));
		case SW_BEND:
			return dir_tick_ccw(dir_tick_ccw(prev_dir));
		default:
			FAIL("impossible combination");
		}
	case SE_BEND:
		switch (curr) {
		case VERTICAL:
			return dir_tick_ccw(prev_dir);
		case HORIZONTAL:
			return dir_tick_cw(prev_dir);
		case NW_BEND:
			return prev_dir;
		case SW_BEND:
			return dir_tick_cw(dir_tick_cw(prev_dir));
		case NE_BEND:
			return dir_tick_ccw(dir_tick_ccw(prev_dir));
		default:
			FAIL("impossible combination");
		}
	default:
		FAIL("impossible combination");
	}
}

static void solution2(const char *const input, char *const output) {
	int width, height;
	struct point start;
	enum pipe *maze = parse_input(input, &width, &height, &start);
	
	struct tile_info *maze_info = malloc(sizeof(*maze_info)*width*height);
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			IDX_RAW(maze_info, i, j, width).is_pipe = false;
			IDX_RAW(maze_info, i, j, width).is_outside = false;
			IDX_RAW(maze_info, i, j, width).is_inside = false;
			IDX_RAW(maze_info, i, j, width).visiting = false;
			IDX_RAW(maze_info, i, j, width).out = -1;
		}
	}

	find_loop(maze, start, width, height, maze_info);

	// find an outside tile
	struct point outside;
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			if (!IDX_RAW(maze_info, i, j, width).is_pipe) {
				outside.x = i;
				outside.y = j;
				IDX_RAW(maze_info, i, j, width).is_inside = false;
				IDX_RAW(maze_info, i, j, width).is_outside = true;
				goto done;
			}
		}
	}
 done:

	// find any straight pipe that touches the outside
	for (int i=0;; i++) {
		outside.x += 1;
		if (outside.x >= width) {
			outside.x = 0;
			outside.y += 1;
		}
		if (IDX(maze_info, outside, width).is_pipe) {
			enum pipe p = IDX(maze, outside, width);
			if (p == VERTICAL || p == HORIZONTAL) {
				enum direction d;
				if (p == VERTICAL) {
					d = WEST;
				} else {
					d = NORTH;
				}
				IDX(maze_info, outside, width).out = d;
				break;
			} else {
				outside.x = 0;
				outside.y += 1;
			}
		} else {
			IDX(maze_info, outside, width).is_inside = false;
			IDX(maze_info, outside, width).is_outside = true;
		}
	}

	// follow the loop marking unknown pipes based on the pipe we
	// come from
	struct point curr, prev;
	pipe_next(maze, outside, &curr, &prev, width, height);
	if (IDX(maze, curr, width) == START) {
		curr = prev;
	}
	prev = outside;
	while (!point_eq(curr, outside)) {
		enum direction dir = follow_out_direction(IDX(maze, curr, width),
							  IDX(maze, prev, width),
							  IDX(maze_info, prev, width).out);
		DBG("curr -> %d,%d '%c' %s", curr.x, curr.y, IDX(maze, curr, width), dirname[dir]);
		IDX(maze_info, curr, width).out = dir;

		// advance
		struct point n1, n2;
		pipe_next(maze, curr, &n1, &n2, width, height);
		struct point tmp = curr;
		if (point_eq(prev, n1)) {
			curr = n2;
		} else {
			curr = n1;
		}
		prev = tmp;
	}
	
	// mark all outside tiles, they are outside if they are next
	// to an outside tile or to a pipe tile with their out
	// direction pointing towards them
	int res = 0;
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			struct point p;
			p.x = i;
			p.y = j;
			if (is_inside(maze, maze_info, p, width, height)) {
				res += 1;
			}
		}
	}

#ifdef DEBUG
	for (int j=0; j<height; j++) {
		for (int i=0; i<width; i++) {
			struct tile_info info = IDX_RAW(maze_info, i, j, width);
			
			char c;
			if (info.is_pipe) {
				switch (info.out) {
				case NORTHWEST:
					c = '/'; break;
				case NORTH:
					c = '^'; break;
				case NORTHEAST:
					c = '\\'; break;
				case EAST:
					c = '>'; break;
				case SOUTHEAST:
					c = '/'; break;
				case SOUTH:
					c = 'v'; break;
				case SOUTHWEST:
					c = '\\'; break;
				case WEST:
					c = '<'; break;
				default:
					c = 'P'; break;
				}
			} else {
				if (info.is_outside) {
					c = 'O';
				} else if (info.is_inside) {
					c = 'I';
				} else {
					c = '.';
				}
			}
			fprintf(stderr, "%c", c);
		}
		fprintf(stderr, "\n");
	}
#endif

	free(maze);
	free(maze_info);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
