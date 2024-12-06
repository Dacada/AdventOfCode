#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define IDX(array, x, y, width) ((array)[(x) + (y) * (width)])

static char *parse_input(const char *input, int *width, int *height) {
  int len = 0;
  int cap = 16;
  char *list = malloc(sizeof(*list) * cap);

  int w = 0;
  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
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

static bool *init_positions(int width, int height) {
  bool *positions;
  size_t s = sizeof(*positions) * width * height;
  positions = malloc(s);
  memset(positions, 0, s);
  return positions;
}

static void advance(const char *map, bool **current_ptr, int width, int height) {
  bool *current = *current_ptr;
  bool *next = init_positions(width, height);

  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      if (!IDX(current, i, j, width)) {
        continue;
      }

      for (int d = 0; d < 4; d++) {
        int x = i;
        int y = j;
        switch (d) {
        case 0:
          y--;
          break;
        case 1:
          x--;
          break;
        case 2:
          y++;
          break;
        case 3:
          x++;
          break;
        }

        if (x < 0 || x >= width || y < 0 || y >= height) {
          continue;
        }
        if (IDX(map, x, y, width) == '#') {
          continue;
        }

        IDX(next, x, y, width) = true;
      }
    }
  }

  free(current);
  *current_ptr = next;
}

static void visualize(const char *map, const bool *positions, int width, int height) {
#ifdef DEBUG
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      char c;
      if (IDX(positions, i, j, width)) {
        c = 'O';
      } else {
        c = IDX(map, i, j, width);
      }
      fputc(c, stderr);
    }
    fputc('\n', stderr);
  }
  fputc('\n', stderr);
#else
  (void)map;
  (void)positions;
  (void)width;
  (void)height;
#endif
}

static bool *init_positions_from_map(char *map, int width, int height) {
  int init_x = -1;
  int init_y = -1;
  for (int j = 0; j < height; j++) {
    for (int i = 0; i < width; i++) {
      char c = IDX(map, i, j, width);
      if (c == 'S') {
        IDX(map, i, j, width) = '.';
        init_x = i;
        init_y = j;
        goto end;
      }
    }
  }

end:
  bool *positions = init_positions(width, height);
  IDX(positions, init_x, init_y, width) = true;
  return positions;
}

static int count_positions(const bool *positions, int width, int height) {
  int res = 0;
  for (int i = 0; i < width * height; i++) {
    if (positions[i]) {
      res += 1;
    }
  }
  return res;
}

static void solution1(const char *const input, char *const output) {
  int width, height;
  char *map = parse_input(input, &width, &height);
  bool *positions = init_positions_from_map(map, width, height);

  for (int i = 0; i < 64; i++) {
    advance(map, &positions, width, height);
    visualize(map, positions, width, height);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count_positions(positions, width, height));
  free(map);
  free(positions);
}

static long run_map_and_count(const char *map, int x, int y, int width, int height, int steps) {
  bool *positions = init_positions(width, height);
  IDX(positions, x, y, width) = true;

  for (int i = 0; i < steps; i++) {
    advance(map, &positions, width, height);
  }

  visualize(map, positions, width, height);
  long res = count_positions(positions, width, height);
  free(positions);
  return res;
}

static void solution2(const char *const input, char *const output) {
  int width, height;
  char *map = parse_input(input, &width, &height);
  ASSERT(IDX(map, width / 2, height / 2, width) == 'S', "start should be in the middle");
  IDX(map, width / 2, height / 2, width) = '.';

  /* Map shape is roughly:
   *
   *  _65_ 1 _65_
   * /    \ /    \
   * ######.######\
   * #####...##### |
   * ####.....#### |65
   * ###..#.#..### |
   * ##..##.##..## |
   * #..###.###..#/
   * .............  1
   * #..###.###..#\
   * ##..##.##..## |
   * ###..#.#..### |65
   * ####.....#### |
   * #####...##### |
   * ######.######/
   *
   * Where the blocks of # are random fields of # or .
   */

  // It's clear that this map shape guarantees that parallel
  // universes will be reached simultaneouly. The result is that
  // the expansion will always look like a rhombus and will be
  // uniform.

  // 26501365 = 202300 * 131 + 65 (hehe, 2023)

  /* Steps: 0
   * #######.#######
   * ######...######
   * #####.....#####
   * ####..#.#..####
   * ###..##.##..###
   * ##..###.###..##
   * #..####.####..#
   * .......o.......
   * #..####.####..#
   * ##..###.###..##
   * ###..##.##..###
   * ####..#.#..####
   * #####.....#####
   * ######...######
   * #######.#######
   */

  /* Steps: 1
   * #######.#######
   * ######...######
   * #####.....#####
   * ####..#.#..####
   * ###..##.##..###
   * ##..###.###..##
   * #..####o####..#
   * ......o.o......
   * #..####o####..#
   * ##..###.###..##
   * ###..##.##..###
   * ####..#.#..####
   * #####.....#####
   * ######...######
   * #######.#######
   */

  /* Steps: 65
   * #######o#######
   * ######o.o######
   * #####o.o.o#####
   * ####o.#.#.o####
   * ###o.##o##.o###
   * ##o.###.###.o##
   * #o.####o####.o#
   * o.o.o.o.o.o.o.o
   * #o.####o####.o#
   * ##o.###.###.o##
   * ###o.##o##.o###
   * ####o.#.#.o####
   * #####o.o.o#####
   * ######o.o######
   * #######o#######
   */

  // After 65 + 1 steps have elapsed, we have reached a parallel
  // universe on all 4 cardinals simultaneously

  /* Steps: 65 + 1
   *                #######.#######
   *                ######...######
   *                #####.....#####
   *                ####..#.#..####
   *                ###..##.##..###
   *                ##..###.###..##
   *                #..####.####..#
   *                .......o.......
   *                #..####.####..#
   *                ##..###.###..##
   *                ###..##.##..###
   *                ####..#.#..####
   *                #####.....#####
   *                ######...######
   *                #######o#######
   * #######.##############.##############.#######
   * ######...############.o.############...######
   * #####.....##########.o.o.##########.....#####
   * ####..#.#..########.o#o#o.########..#.#..####
   * ###..##.##..######.o##.##o.######..##.##..###
   * ##..###.###..####.o###o###o.####..###.###..##
   * #..####.####..##.o####.####o.##..####.####..#
   * ..............o.o.o.o.o.o.o.o.o..............
   * #..####.####..##.o####.####o.##..####.####..#
   * ##..###.###..####.o###o###o.####..###.###..##
   * ###..##.##..######.o##.##o.######..##.##..###
   * ####..#.#..########.o#o#o.########..#.#..####
   * #####.....##########.o.o.##########.....#####
   * ######...############.o.############...######
   * #######.##############.##############.#######
   *                #######o#######
   *                ######...######
   *                #####.....#####
   *                ####..#.#..####
   *                ###..##.##..###
   *                ##..###.###..##
   *                #..####.####..#
   *                .......o.......
   *                #..####.####..#
   *                ##..###.###..##
   *                ###..##.##..###
   *                ####..#.#..####
   *                #####.....#####
   *                ######...######
   *                #######.#######
   */

  // After 1 step has elapsed on the first layer of parallel
  // dimensions, we have just reached them. Let's see what it
  // looks like after 131 steps have elapsed on the first layer
  // of parallel dimensions (131 + 65 total)

  /* Steps: 65 + 131
   *                #######o#######
   *                ######...######
   *                #####..o..#####
   *                ####..#.#..####
   *                ###..##o##..###
   *                ##..###.###..##
   *                #o.####o####.o#
   *                o.o.o.o.o.o.o.o
   *                #o.####o####.o#
   *                ##o.###.###.o##
   *                ###o.##o##.o###
   *                ####o.#.#.o####
   *                #####o.o.o#####
   *                ######o.o######
   *                #######o#######
   * #######o##############.##############o#######
   * ######o.o############.o.############o.o######
   * #####..o.o##########.o.o.##########o.o..#####
   * ####..#.#.o########.o#o#o.########o.#.#..####
   * ###..##o##.o######.o##.##o.######o.##o##..###
   * ##..###.###.o####.o###o###o.####o.###.###..##
   * #..####o####.o##.o####.####o.##o.####o####..#
   * o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o.o
   * #..####o####.o##.o####.####o.##o.####o####..#
   * ##..###.###.o####.o###o###o.####o.###.###..##
   * ###..##o##.o######.o##.##o.######o.##o##..###
   * ####..#.#.o########.o#o#o.########o.#.#..####
   * #####..o.o##########.o.o.##########o.o..#####
   * ######o.o############.o.############o.o######
   * #######o##############.##############o#######
   *                #######o#######
   *                ######o.o######
   *                #####o.o.o#####
   *                ####o.#.#.o####
   *                ###o.##o##.o###
   *                ##o.###.###.o##
   *                #o.####o####.o#
   *                o.o.o.o.o.o.o.o
   *                #o.####o####.o#
   *                ##..###.###..##
   *                ###..##o##..###
   *                ####..#.#..####
   *                #####..o..#####
   *                ######...######
   *                #######o#######
   */

  // There is more, when you zoom it out, such as there being
  // two kinds of diagonals at the edge. But I am getting lazy.

  ASSERT(width == height, "need square grid");
  // we also assume the grid is as described above but im not
  // checking that in the code, too much work X_X

  const long steps = 26501365;
  long radius = steps / width;

  // We calculate the centered square number iteratively to
  // easily get odds and evens as well
  long inner_odd_parallel_universes = 1;
  long inner_even_parallel_universes = 0;
  {
    for (int i = 1; i < radius; i++) {
      if (i % 2 == 0) {
        inner_odd_parallel_universes += i * 4;
      } else {
        inner_even_parallel_universes += i * 4;
      }
    }
  }
  ASSERT(inner_odd_parallel_universes + inner_even_parallel_universes ==
             (radius - 1) * (radius - 1) * 2 + (radius - 1) * 2 + 1,
         "sanity check");

  long inner_odd = run_map_and_count(map, width / 2, height / 2, width, height, width);
  long inner_even = run_map_and_count(map, width / 2, height / 2, width, height, width + 1);
  (void)inner_odd;
  (void)inner_even;

  long north = run_map_and_count(map, width / 2, 0, width, height, width - 1);
  long west = run_map_and_count(map, 0, height / 2, width, height, width - 1);
  long south = run_map_and_count(map, width / 2, height - 1, width, height, width - 1);
  long east = run_map_and_count(map, width - 1, height / 2, width, height, width - 1);

  long lim1 = (steps - width / 2 - height / 2 - 2) % (width + height);
  long lim2 = (steps - width / 2 - height / 2 - height - 2) % (width + height);
  long nw1 = run_map_and_count(map, 0, 0, width, height, lim1);
  long ne1 = run_map_and_count(map, width - 1, 0, width, height, lim1);
  long sw1 = run_map_and_count(map, 0, height - 1, width, height, lim1);
  long se1 = run_map_and_count(map, width - 1, height - 1, width, height, lim1);
  long nw2 = run_map_and_count(map, 0, 0, width, height, lim2);
  long ne2 = run_map_and_count(map, width - 1, 0, width, height, lim2);
  long sw2 = run_map_and_count(map, 0, height - 1, width, height, lim2);
  long se2 = run_map_and_count(map, width - 1, height - 1, width, height, lim2);

  long res = inner_odd * inner_odd_parallel_universes + inner_even * inner_even_parallel_universes +
             (nw1 + ne1 + se1 + sw1) * (radius - 1) + (nw2 + ne2 + se2 + sw2) * radius + north + south + east + west;

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
  free(map);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
