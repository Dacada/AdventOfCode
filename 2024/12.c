#include <aoclib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void get_fencing_price(struct aoc_point start, const char *plots, bool *searched, struct aoc_dynarr *to_search,
                              int height, int width, int *area_ptr, int *perimeter_ptr, int *sides_ptr) {
  char name = plots[AOC_2D_IDX(start.x, start.y, width)];

  struct aoc_dynarr stack;
  aoc_dynarr_init(&stack, sizeof(struct aoc_point), 16);
  struct aoc_point *p = aoc_dynarr_grow(&stack, 1);
  *p = start;

  int area = 0;
  int perimeter = 0;
  int sides = 0;

  while (stack.len > 0) {
    p = aoc_dynarr_shrink(&stack, 1);
    struct aoc_point current = *p;
    if (searched[AOC_2D_IDX(current.x, current.y, width)]) {
      continue;
    }
    area++;
    searched[AOC_2D_IDX(current.x, current.y, width)] = true;

    for (int i = 0; i < 4; i++) {
      struct aoc_point next = aoc_point_move(current, i);
      if (!aoc_point_in_limits(next, width, height)) {
        perimeter++;
        continue;
      }
      if (plots[AOC_2D_IDX(next.x, next.y, width)] != name) {
        perimeter++;
        p = aoc_dynarr_grow(to_search, 1);
        *p = next;
        continue;
      }
      p = aoc_dynarr_grow(&stack, 1);
      *p = next;
    }

    // Corner detection
    struct aoc_point np = aoc_point_move(current, AOC_DIRECTION_NORTH);
    struct aoc_point ep = aoc_point_move(current, AOC_DIRECTION_EAST);
    struct aoc_point sp = aoc_point_move(current, AOC_DIRECTION_SOUTH);
    struct aoc_point wp = aoc_point_move(current, AOC_DIRECTION_WEST);
    char nc = aoc_point_in_limits(np, width, height) ? plots[AOC_2D_IDX(np.x, np.y, width)] : '\0';
    char ec = aoc_point_in_limits(ep, width, height) ? plots[AOC_2D_IDX(ep.x, ep.y, width)] : '\0';
    char sc = aoc_point_in_limits(sp, width, height) ? plots[AOC_2D_IDX(sp.x, sp.y, width)] : '\0';
    char wc = aoc_point_in_limits(wp, width, height) ? plots[AOC_2D_IDX(wp.x, wp.y, width)] : '\0';
    bool north = nc == name;
    bool east = ec == name;
    bool south = sc == name;
    bool west = wc == name;

    // AAA BBB BBB
    // AAA BBB BBB
    // AAA BBB BBB
    //
    // AAA BBB BBB
    // AAA BBB BBB
    // AAA BBB BBB
    //
    // AAA AAA AAA
    // AAA AAA AAA
    // AAA AAA AAA

    // We're outside the L
    if (north && east && !south && !west) {
      sides++;
    }
    if (east && south && !west && !north) {
      sides++;
    }
    if (south && west && !north && !east) {
      sides++;
    }
    if (west && north && !east && !south) {
      sides++;
    }

    // We're inside the L
    if (north && east && south && west) {
      struct aoc_point pnw = aoc_point_move(aoc_point_move(current, AOC_DIRECTION_NORTH), AOC_DIRECTION_WEST);
      struct aoc_point pne = aoc_point_move(aoc_point_move(current, AOC_DIRECTION_NORTH), AOC_DIRECTION_EAST);
      struct aoc_point psw = aoc_point_move(aoc_point_move(current, AOC_DIRECTION_SOUTH), AOC_DIRECTION_WEST);
      struct aoc_point pse = aoc_point_move(aoc_point_move(current, AOC_DIRECTION_SOUTH), AOC_DIRECTION_EAST);
      char cnw = aoc_point_in_limits(pnw, width, height) ? plots[AOC_2D_IDX(pnw.x, pnw.y, width)] : '\0';
      char cne = aoc_point_in_limits(pne, width, height) ? plots[AOC_2D_IDX(pne.x, pne.y, width)] : '\0';
      char csw = aoc_point_in_limits(psw, width, height) ? plots[AOC_2D_IDX(psw.x, psw.y, width)] : '\0';
      char cse = aoc_point_in_limits(pse, width, height) ? plots[AOC_2D_IDX(pse.x, pse.y, width)] : '\0';
      bool northwest = cnw == name;
      bool northeast = cne == name;
      bool southwest = csw == name;
      bool southeast = cse == name;
      if (!northwest && northeast && southwest && southeast) {
        sides++;
      }
      if (northwest && !northeast && southwest && southeast) {
        sides++;
      }
      if (northwest && northeast && !southwest && southeast) {
        sides++;
      }
      if (northwest && northeast && southwest && !southeast) {
        sides++;
      }
    }
  }

  aoc_dynarr_free(&stack);
  *area_ptr = area;
  *perimeter_ptr = perimeter;
  *sides_ptr = sides;
  DBG("%c: a=%d p=%d s=%d", name, area, perimeter, sides);
}

static void solution(const char *input, char *const output, bool use_sides) {
  int height, width;
  char *plots = aoc_parse_grid_chars(&input, &height, &width);

  // scale x3 the grid to make it easier to find corners for part 2
  char *newplots = aoc_malloc(sizeof(*newplots) * height * 3 * width * 3);
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      char c = plots[AOC_2D_IDX(i, j, width)];
      for (int k = 0; k < 3; k++) {
        for (int l = 0; l < 3; l++) {
          newplots[AOC_2D_IDX(i * 3 + k, j * 3 + l, width * 3)] = c;
        }
      }
    }
  }
  free(plots);
  plots = newplots;
  width *= 3;
  height *= 3;

  struct aoc_dynarr to_search;
  aoc_dynarr_init(&to_search, sizeof(struct aoc_point), 16);
  struct aoc_point *p = aoc_dynarr_grow(&to_search, 1);
  p->x = 0;
  p->y = 0;

  bool *searched = aoc_malloc(sizeof(*searched) * height * width);
  memset(searched, 0, sizeof(*searched) * height * width);

  int total = 0;
  for (int i = 0; i < to_search.len; i++) {
    struct aoc_point start = AOC_DYNARR_IDX(to_search, i, struct aoc_point);
    if (searched[AOC_2D_IDX(start.x, start.y, width)]) {
      continue;
    }

    int area, perimeter, sides;
    get_fencing_price(start, plots, searched, &to_search, height, width, &area, &perimeter, &sides);
    if (use_sides) {
      total += area / 9 * sides;
    } else {
      total += area / 9 * perimeter / 3;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(plots);
  aoc_dynarr_free(&to_search);
  free(searched);
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
