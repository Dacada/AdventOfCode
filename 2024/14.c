#include <aoclib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define WIDTH 101
#define HEIGHT 103

struct robot {
  struct aoc_point position;
  struct aoc_point speed;
};

static void parse_robot(const char **input, struct robot *robot) {
  const char *txt = "p=";
  aoc_expect_text(input, txt, strlen(txt));
  robot->position.x = aoc_parse_int(input);

  txt = ",";
  aoc_expect_text(input, txt, strlen(txt));
  robot->position.y = aoc_parse_int(input);

  txt = " v=";
  aoc_expect_text(input, txt, strlen(txt));
  bool neg = **input == '-';
  if (neg) {
    *input += 1;
  }
  robot->speed.x = aoc_parse_int(input);
  if (neg) {
    robot->speed.x = -robot->speed.x;
  }

  txt = ",";
  aoc_expect_text(input, txt, strlen(txt));
  neg = **input == '-';
  if (neg) {
    *input += 1;
  }
  robot->speed.y = aoc_parse_int(input);
  if (neg) {
    robot->speed.y = -robot->speed.y;
  }
}

static struct robot *parse_input(const char *input, int *nrobots) {
  struct aoc_dynarr arr;
  aoc_dynarr_init(&arr, sizeof(struct robot), 16);

  while (*input != '\0') {
    parse_robot(&input, aoc_dynarr_grow(&arr, 1));
    while (*input == '\n') {
      input++;
    }
  }

  *nrobots = arr.len;
  return arr.data;
}

static void robot_advance(struct robot *robot) {
  robot->position.x = aoc_modulo_int(robot->position.x + robot->speed.x, WIDTH);
  robot->position.y = aoc_modulo_int(robot->position.y + robot->speed.y, HEIGHT);
}

static int advance_and_count_quadrants(struct robot *robots, int nrobots, int steps) {
  int q1 = 0;
  int q2 = 0;
  int q3 = 0;
  int q4 = 0;
  for (int i = 0; i < nrobots; i++) {
    for (int j = 0; j < steps; j++) {
      robot_advance(&robots[i]);
    }

    int x = robots[i].position.x;
    int y = robots[i].position.y;
    if (x < WIDTH / 2 && y < HEIGHT / 2) {
      q1++;
    } else if (x > WIDTH / 2 && y < HEIGHT / 2) {
      q2++;
    } else if (x < WIDTH / 2 && y > HEIGHT / 2) {
      q3++;
    } else if (x > WIDTH / 2 && y > HEIGHT / 2) {
      q4++;
    }
  }
  return q1 * q2 * q3 * q4;
}

static char *map_robots(const struct robot *robots, int nrobots) {
  char *map = malloc(sizeof(*map) * WIDTH * HEIGHT);
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    map[i] = '.';
  }
  for (int i = 0; i < nrobots; i++) {
    char *c = &map[AOC_2D_IDX(robots[i].position.x, robots[i].position.y, WIDTH)];
    if (*c == '.') {
      *c = '1';
    } else {
      *c += 1;
    }
  }
  return map;
}

#ifdef DEBUG
static void draw_robots(const struct robot *robots, int nrobots) {
  char *map = map_robots(robots, nrobots);
  for (int j = 0; j < HEIGHT; j++) {
    for (int i = 0; i < WIDTH; i++) {
      fputc(map[AOC_2D_IDX(i, j, WIDTH)], stderr);
    }
    fputc('\n', stderr);
  }
  fputc('\n', stderr);
  free(map);
}
#endif

static void solution1(const char *input, char *const output) {
  int nrobots;
  struct robot *robots = parse_input(input, &nrobots);

  int result = advance_and_count_quadrants(robots, nrobots, 100);
#ifdef DEBUG
  draw_robots(robots, nrobots);
#endif

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
  free(robots);
}

static void solution2(const char *input, char *const output) {
  // Doing the funny quadrants thing means calculating spatial entropy! An image
  // with low entropy will have a structured drawing, while random noise will
  // have high entropy!

  int nrobots;
  struct robot *robots = parse_input(input, &nrobots);

  int average = 0;
  int count = 0;
  int greatest_deviation = INT_MIN;
  int second;
  for (second = 1;; second++) {
    int entropy = advance_and_count_quadrants(robots, nrobots, 1);
    average = average + (entropy - average) / (count + 1);
    count++;
    int deviation = AOC_ABS(entropy - average);
    if (deviation > greatest_deviation) {
      greatest_deviation = deviation;
      // This is a clue, but not enough to be definitive
      // We're gonna map out the robots and search for straight lines
      char *map = map_robots(robots, nrobots);
      int largest_run = INT_MIN;
      for (int j = 0; j < HEIGHT; j++) {
        int run = 0;
        for (int i = 0; i < WIDTH; i++) {
          if (map[AOC_2D_IDX(i, j, WIDTH)] == '1') {
            run++;
            if (run > largest_run) {
              largest_run = run;
            }
          } else {
            run = 0;
          }
        }
      }
      free(map);
      if (largest_run > 25) {
        break;
      }
    }
  }
  DBG("average: %d", average);
  DBG("count:   %d", count);
  DBG("max dev: %d", greatest_deviation);

#ifdef DEBUG
  draw_robots(robots, nrobots);
#endif

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", second);
  free(robots);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
