#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define CUBEDIM 50

enum instruction_type {
  INSTR_ADVANCE,
  INSTR_ROTATE,
};

enum direction {
  DIR_RIGHT,
  DIR_DOWN,
  DIR_LEFT,
  DIR_UP,
};

struct state {
  int x;
  int y;
  enum direction facing;
};

struct instruction {
  enum instruction_type type;
  union {
    int advance;
    enum direction rotate;
  };
};

static enum direction parse_rot(const char **input) {
  if (**input == 'L') {
    *input += 1;
    return DIR_LEFT;
  } else if (**input == 'R') {
    *input += 1;
    return DIR_RIGHT;
  } else {
    FAIL("parse error '%c'", **input);
  }
}

static int parse_int(const char **input) {
  int r = 0;
  while (isdigit(**input)) {
    r *= 10;
    r += **input - '0';
    *input += 1;
  }
  return r;
}

static struct instruction parse_instr(const char **input) {
  struct instruction instr;
  if (isdigit(**input)) {
    instr.type = INSTR_ADVANCE;
    instr.advance = parse_int(input);
  } else {
    instr.type = INSTR_ROTATE;
    instr.rotate = parse_rot(input);
  }
  return instr;
}

static void parse_input(const char *input, int *width, int *height, char **map, int *len, struct instruction **instr) {
  *width = 0;
  *height = 0;
  int w = 0;
  for (int i = 0;; i++) {
    if (input[i] == '\n') {
      *height += 1;
      if (w > *width) {
        *width = w;
      }
      w = 0;
      if (input[i + 1] == '\n') {
        break;
      } else {
        continue;
      }
    } else {
      w++;
    }
  }

  *map = malloc(sizeof(**map) * *width * *height);
  for (int i = 0; i < *width * *height; i++) {
    (*map)[i] = ' ';
  }

  int i = 0;
  int j = 0;
  while (*input != '\n') {
    char c = *input;
    (*map)[j * *width + i] = c;
    i++;

    input++;
    if (*input == '\n') {
      i = 0;
      j++;
      input++;
    }
  }
  input++;

  *len = 0;
  int cap = 16;
  *instr = malloc(sizeof(*instr) * cap);
  while (*input != '\0') {
    if (*len >= cap) {
      cap *= 2;
      *instr = realloc(*instr, sizeof(*instr) * cap);
    }
    (*instr)[*len] = parse_instr(&input);
    *len += 1;
    while (*input == '\n') {
      input++;
    }
  }
}

#define CURRENT map[state->y * width + state->x]
#define BOUND                                                                                                          \
  if (state->x < 0) {                                                                                                  \
    state->x = width - 1;                                                                                              \
  }                                                                                                                    \
  if (state->x >= width) {                                                                                             \
    state->x = 0;                                                                                                      \
  }                                                                                                                    \
  if (state->y < 0) {                                                                                                  \
    state->y = height - 1;                                                                                             \
  }                                                                                                                    \
  if (state->y >= height) {                                                                                            \
    state->y = 0;                                                                                                      \
  }
#define WALL                                                                                                           \
  if (CURRENT == '#') {                                                                                                \
    state->x = origx;                                                                                                  \
    state->y = origy;                                                                                                  \
    return;                                                                                                            \
  }
static void advance_plane(int n, struct state *const state, const char *map, const int width, const int height) {
  int advx = 0;
  int advy = 0;
  switch (state->facing) {
  case DIR_RIGHT:
    advx = 1;
    break;
  case DIR_DOWN:
    advy = 1;
    break;
  case DIR_LEFT:
    advx = -1;
    break;
  case DIR_UP:
    advy = -1;
    break;
  }

  for (int i = 0; i < n; i++) {
    int origx = state->x;
    int origy = state->y;

    state->x += advx;
    state->y += advy;

    BOUND;
    WALL;

    while (CURRENT == ' ') {
      state->x += advx;
      state->y += advy;
      BOUND;
    }
    WALL;

    DBG("%c", ">v<^"[state->facing]);
  }
}
#undef WALL
#undef BOUND

#define DOADV                                                                                                          \
  switch (state->facing) {                                                                                             \
  case DIR_RIGHT:                                                                                                      \
    state->x++;                                                                                                        \
    break;                                                                                                             \
  case DIR_DOWN:                                                                                                       \
    state->y++;                                                                                                        \
    break;                                                                                                             \
  case DIR_LEFT:                                                                                                       \
    state->x--;                                                                                                        \
    break;                                                                                                             \
  case DIR_UP:                                                                                                         \
    state->y--;                                                                                                        \
    break;                                                                                                             \
  }
static void advance_cube(int n, struct state *const state, const char *map, const int width, const int height) {
#define XQUAD (state->x / CUBEDIM)
#define YQUAD (state->y / CUBEDIM)

  for (int i = 0; i < n; i++) {
    struct state orig = *state;

    if (XQUAD == 0 && YQUAD == 0) {
      FAIL("invalid position");
    } else if (XQUAD == 1 && YQUAD == 0) {
      DOADV;
      ASSERT(state->x >= 0 && state->x < width && state->y < height, "invalid position");
      if (state->y < 0) {
        state->facing = DIR_RIGHT;
        state->y = state->x - CUBEDIM + 3 * CUBEDIM;
        state->x = 0 * CUBEDIM;
        ASSERT(XQUAD == 0 && YQUAD == 3, "invalid position");
      } else if (XQUAD == 0) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_RIGHT;
        state->x = 0 * CUBEDIM;
        state->y = (CUBEDIM - state->y - 1) + 2 * CUBEDIM;
        ASSERT(XQUAD == 0 && YQUAD == 2, "invalid position");
      } else if (XQUAD == 2) {
        ASSERT(XQUAD == 2 && YQUAD == 0, "invalid position");
      } else if (YQUAD == 1) {
        ASSERT(XQUAD == 1 && YQUAD == 1, "invalid position");
      } else {
        ASSERT(XQUAD == 1 && YQUAD == 0, "invalid position");
      }
    } else if (XQUAD == 2 && YQUAD == 0) {
      DOADV;
      ASSERT(state->x >= 0 && state->y < height, "invalid position");
      if (state->y < 0) {
        state->facing = DIR_UP;
        state->x -= 2 * CUBEDIM;
        state->y = 4 * CUBEDIM - 1;
        ASSERT(XQUAD == 0 && YQUAD == 3, "invalid position");
      } else if (state->x >= width) {
        state->facing = DIR_LEFT;
        state->x = 2 * CUBEDIM - 1;
        state->y = (CUBEDIM - state->y - 1) + 2 * CUBEDIM;
        ASSERT(XQUAD == 1 && YQUAD == 2, "invalid position");
      } else if (YQUAD == 1) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_LEFT;
        state->y = state->x - 2 * CUBEDIM + 1 * CUBEDIM;
        state->x = 2 * CUBEDIM - 1;
        ASSERT(XQUAD == 1 && YQUAD == 1, "invalid position");
      } else if (XQUAD == 1) {
        ASSERT(XQUAD == 1 && YQUAD == 0, "invalid position");
      } else {
        ASSERT(XQUAD == 2 && YQUAD == 0, "invalid position");
      }
    } else if (XQUAD == 0 && YQUAD == 1) {
      FAIL("invalid position");
    } else if (XQUAD == 1 && YQUAD == 1) {
      DOADV;
      ASSERT(state->x >= 0 && state->x < width && state->y >= 0 && state->y < height, "invalid position");
      if (XQUAD == 0) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_DOWN;
        state->x = state->y - 1 * CUBEDIM;
        state->y = 2 * CUBEDIM;
        ASSERT(XQUAD == 0 && YQUAD == 2, "invalid position");
      } else if (XQUAD == 2) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_UP;
        state->x = state->y - 1 * CUBEDIM + 2 * CUBEDIM;
        state->y = 1 * CUBEDIM - 1;
        ASSERT(XQUAD == 2 && YQUAD == 0, "invalid position");
      } else if (YQUAD == 0) {
        ASSERT(XQUAD == 1 && YQUAD == 0, "invalid position");
      } else if (YQUAD == 2) {
        ASSERT(XQUAD == 1 && YQUAD == 2, "invalid position");
      } else {
        ASSERT(XQUAD == 1 && YQUAD == 1, "invalid position");
      }
    } else if (XQUAD == 2 && YQUAD == 1) {
      FAIL("invalid position");
    } else if (XQUAD == 0 && YQUAD == 2) {
      DOADV;
      ASSERT(state->x < width && state->y >= 0 && state->y < height, "invalid position");
      if (state->x < 0) {
        state->facing = DIR_RIGHT;
        state->x = 1 * CUBEDIM;
        state->y = CUBEDIM - (state->y - 2 * CUBEDIM) - 1;
        ASSERT(XQUAD == 1 && YQUAD == 0, "invalid position");
      } else if (YQUAD == 1) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_RIGHT;
        state->y = state->x + 1 * CUBEDIM;
        state->x = 1 * CUBEDIM;
        ASSERT(XQUAD == 1 && YQUAD == 1, "invalid position");
      } else if (XQUAD == 1) {
        ASSERT(XQUAD == 1 && YQUAD == 2, "invalid position");
      } else if (YQUAD == 3) {
        ASSERT(XQUAD == 0 && YQUAD == 3, "invalid position");
      } else {
        ASSERT(XQUAD == 0 && YQUAD == 2, "invalid position");
      }
    } else if (XQUAD == 1 && YQUAD == 2) {
      DOADV;
      ASSERT(state->x >= 0 && state->x < width && state->y >= 0 && state->y < height, "invalid position");
      if (XQUAD == 0) {
        ASSERT(XQUAD == 0 && YQUAD == 2, "invalid position");
      } else if (XQUAD == 2) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_LEFT;
        state->x = 3 * CUBEDIM - 1;
        state->y = CUBEDIM - (state->y - 2 * CUBEDIM) - 1;
        ASSERT(XQUAD == 2 && YQUAD == 0, "invalid position");
      } else if (YQUAD == 1) {
        ASSERT(XQUAD == 1 && YQUAD == 1, "invalid position");
      } else if (YQUAD == 3) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_LEFT;
        state->y = state->x - 1 * CUBEDIM + 3 * CUBEDIM;
        state->x = 1 * CUBEDIM - 1;
        ASSERT(XQUAD == 0 && YQUAD == 3, "invalid position");
      } else {
        ASSERT(XQUAD == 1 && YQUAD == 2, "invalid position");
      }
    } else if (XQUAD == 2 && YQUAD == 2) {
      FAIL("invalid position");
    } else if (XQUAD == 0 && YQUAD == 3) {
      DOADV;
      ASSERT(state->x < width && state->y >= 0, "invalid position");
      if (state->x < 0) {
        state->facing = DIR_DOWN;
        state->x = state->y - 3 * CUBEDIM + 1 * CUBEDIM;
        state->y = 0;
        ASSERT(XQUAD == 1 && YQUAD == 0, "invalid position");
      } else if (state->y >= height) {
        state->facing = DIR_DOWN;
        state->x += 2 * CUBEDIM;
        state->y = 0;
        ASSERT(XQUAD == 2 && YQUAD == 0, "invalid position");
      } else if (XQUAD == 1) {
        ASSERT(CURRENT == ' ', "unexpected tile");
        state->facing = DIR_UP;
        state->x = state->y - 3 * CUBEDIM + 1 * CUBEDIM;
        state->y = 3 * CUBEDIM - 1;
        ASSERT(XQUAD == 1 && YQUAD == 2, "invalid position");
      } else if (YQUAD == 2) {
        ASSERT(XQUAD == 0 && YQUAD == 2, "invalid position");
      } else {
        ASSERT(XQUAD == 0 && YQUAD == 3, "invalid position");
      }
    } else if (XQUAD == 1 && YQUAD == 3) {
      FAIL("invalid position");
    } else if (XQUAD == 2 && YQUAD == 3) {
      FAIL("invalid position");
    } else {
      FAIL("invalid position");
    }
    ASSERT(state->x >= 0 && state->x < width && state->y >= 0 && state->y < height, "invalid position");

    if (CURRENT == '#') {
      *state = orig;
      return;
    } else {
      ASSERT(CURRENT == '.', "unexpected tile");
    }
  }
#undef XQUAD
#undef YQUAD
}
#undef DOADV
#undef CURRENT

static void follow_instructions(struct state *const state, const char *map, const struct instruction *instr,
                                const int width, const int height, const int len, bool cube) {
  void (*advance)(int, struct state *, const char *, int, int);
  if (cube) {
    advance = advance_cube;
  } else {
    advance = advance_plane;
  }
  for (int i = 0; i < len; i++) {
    const struct instruction *in = instr + i;
    if (in->type == INSTR_ADVANCE) {
      DBG("advance %d", in->advance);
    } else {
      DBG("rotate %s", ((const char *[]){"right", "???", "left"})[in->rotate]);
    }
    switch (in->type) {
    case INSTR_ADVANCE:
      advance(in->advance, state, map, width, height);
      break;
    case INSTR_ROTATE:
      switch (in->rotate) {
      case DIR_LEFT:
        state->facing = (state->facing - 1) % 4;
        break;
      case DIR_RIGHT:
        state->facing = (state->facing + 1) % 4;
        break;
      default:
        FAIL("invalid rotate");
      }
      break;
    default:
      FAIL("invalid instr");
    }
    DBG("%d %d %d", state->y + 1, state->x + 1, state->facing);
  }
}

static inline int password(const struct state *s) { return 1000 * (s->y + 1) + 4 * (s->x + 1) + s->facing; }

static void solution(const char *const input, char *const output, bool cube) {
  int width, height, len;
  char *map;
  struct instruction *instr;
  parse_input(input, &width, &height, &map, &len, &instr);

#ifdef DEBUG
  ASSERT(width == 3 * CUBEDIM && height == 4 * CUBEDIM, "parse error");
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      fputc(map[j * width + i], stderr);
    }
    fputc('\n', stderr);
  }
  fputc('\n', stderr);
  for (int i = 0; i < len; i++) {
    struct instruction *in = &instr[i];
    switch (in->type) {
    case INSTR_ADVANCE:
      fprintf(stderr, "%d", in->advance);
      break;
    case INSTR_ROTATE:
      fputc("R?L?"[in->rotate], stderr);
      break;
    default:
      FAIL("parse error");
    }
  }
  fputc('\n', stderr);
#endif

  struct state state;
  state.y = 0;
  state.facing = DIR_RIGHT;
  for (int i = 0; i < width; i++) {
    if (map[i] != ' ') {
      state.x = i;
      break;
    }
  }
  follow_instructions(&state, map, instr, width, height, len, cube);
  int res = password(&state);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  free(instr);
  free(map);
}

static void solution1(const char *const input, char *const output) { solution(input, output, false); }

static void solution2(const char *const input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
