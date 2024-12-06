#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct blueprint {
  int ore_ore;
  int clay_ore;
  int obsidian_ore;
  int obsidian_clay;
  int geode_ore;
  int geode_obsidian;

  int max_ore;
  int max_clay;
  int max_obsidian;
};

struct state {
  int ore;
  int clay;
  int obsidian;

  int ore_robots;
  int clay_robots;
  int obsidian_robots;
};

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error");
  int r = 0;
  while (isdigit(**input)) {
    r *= 10;
    r += **input - '0';
    *input += 1;
  }
  return r;
}

#define ASSERT_STR(input, str)                                                                                         \
  ASSERT(strncmp(input, str, strlen(str)) == 0, "parse error");                                                        \
  input += strlen(str);

static void parse_line(const char **input, struct blueprint *blueprint) {
  ASSERT_STR(*input, "Blueprint ");
  parse_int(input);
  ASSERT_STR(*input, ": Each ore robot costs ");
  blueprint->ore_ore = parse_int(input);
  ASSERT_STR(*input, " ore. Each clay robot costs ");
  blueprint->clay_ore = parse_int(input);
  ASSERT_STR(*input, " ore. Each obsidian robot costs ");
  blueprint->obsidian_ore = parse_int(input);
  ASSERT_STR(*input, " ore and ");
  blueprint->obsidian_clay = parse_int(input);
  ASSERT_STR(*input, " clay. Each geode robot costs ");
  blueprint->geode_ore = parse_int(input);
  ASSERT_STR(*input, " ore and ");
  blueprint->geode_obsidian = parse_int(input);
  ASSERT_STR(*input, " obsidian.");
}

#undef ASSERT_STR

static void skip_newlines(const char **input) {
  while (**input == '\n') {
    *input += 1;
  }
}

static int parse_input(const char *input, struct blueprint **blueprints) {
  int len = 0;
  int cap = 16;
  *blueprints = malloc(sizeof(**blueprints) * cap);
  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      *blueprints = realloc(*blueprints, sizeof(**blueprints) * cap);
    }
    parse_line(&input, (*blueprints) + len);
    skip_newlines(&input);
    len++;
  }
  return len;
}

enum robots {
  ROBOT_ore,
  ROBOT_clay,
  ROBOT_obsidian,
  ROBOT_geode,
};

static inline bool could_make_robot(enum robots robot, const struct blueprint *blueprint, const struct state *state,
                                    int minutes) {
  switch (robot) {
  case ROBOT_ore:
    return state->ore_robots * minutes + state->ore < blueprint->max_ore * minutes && state->ore >= blueprint->ore_ore;
  case ROBOT_clay:
    return state->clay_robots * minutes + state->clay < blueprint->max_clay * minutes &&
           state->ore >= blueprint->clay_ore;
  case ROBOT_obsidian:
    return state->obsidian_robots * minutes + state->obsidian < blueprint->max_obsidian * minutes &&
           state->ore >= blueprint->obsidian_ore && state->clay >= blueprint->obsidian_clay;
  case ROBOT_geode:
    return state->ore >= blueprint->geode_ore && state->obsidian >= blueprint->geode_obsidian;
  default:
    FAIL("invalid enum");
  }
}

static void robot_gather(struct state *state) {
  state->ore += state->ore_robots;
  state->clay += state->clay_robots;
  state->obsidian += state->obsidian_robots;
}

__attribute__((pure)) static int geods(const struct blueprint *blueprint, const struct state *prev, const int minutes,
                                       bool stop_ore, bool stop_clay, bool stop_obsidian, bool stop_geods) {
  if (minutes == 0) {
    return 0;
  }

  struct state next = *prev;
  robot_gather(&next);
  int best = 0;

  bool make_ore = could_make_robot(ROBOT_ore, blueprint, prev, minutes);
  bool make_clay = could_make_robot(ROBOT_clay, blueprint, prev, minutes);
  bool make_obsidian = could_make_robot(ROBOT_obsidian, blueprint, prev, minutes);
  bool make_geods = could_make_robot(ROBOT_geode, blueprint, prev, minutes);

  if (make_ore && !stop_ore) {
    next.ore -= blueprint->ore_ore;
    next.ore_robots++;
    int n = geods(blueprint, &next, minutes - 1, false, false, false, false);
    next.ore_robots--;
    next.ore += blueprint->ore_ore;
    if (n > best) {
      best = n;
    }
  }

  if (make_clay && !stop_clay) {
    next.ore -= blueprint->clay_ore;
    next.clay_robots++;
    int n = geods(blueprint, &next, minutes - 1, false, false, false, false);
    next.clay_robots--;
    next.ore += blueprint->clay_ore;
    if (n > best) {
      best = n;
    }
  }

  if (make_obsidian && !stop_obsidian) {
    next.ore -= blueprint->obsidian_ore;
    next.clay -= blueprint->obsidian_clay;
    next.obsidian_robots++;
    int n = geods(blueprint, &next, minutes - 1, false, false, false, false);
    next.obsidian_robots--;
    next.ore += blueprint->obsidian_ore;
    next.clay += blueprint->obsidian_clay;
    if (n > best) {
      best = n;
    }
  }

  if (make_geods && !stop_geods) {
    next.ore -= blueprint->geode_ore;
    next.obsidian -= blueprint->geode_obsidian;
    int created_geods = minutes - 1;
    int n = created_geods + geods(blueprint, &next, minutes - 1, false, false, false, false);
    next.ore += blueprint->geode_ore;
    next.obsidian += blueprint->geode_obsidian;
    if (n > best) {
      best = n;
    }
  }

  // do nothing
  int n = geods(blueprint, &next, minutes - 1, make_ore, make_clay, make_obsidian, make_geods);
  if (n > best) {
    best = n;
  }

  return best;
}

static void preprocess(struct blueprint *blueprint) {
  blueprint->max_ore = 0;
  blueprint->max_clay = 0;
  blueprint->max_obsidian = 0;

  blueprint->max_ore = blueprint->ore_ore;
  blueprint->max_clay = blueprint->obsidian_clay;
  blueprint->max_obsidian = blueprint->geode_obsidian;

  if (blueprint->clay_ore > blueprint->max_ore) {
    blueprint->max_ore = blueprint->clay_ore;
  }
  if (blueprint->obsidian_ore > blueprint->max_ore) {
    blueprint->max_ore = blueprint->obsidian_ore;
  }
  if (blueprint->geode_ore > blueprint->max_ore) {
    blueprint->max_ore = blueprint->geode_ore;
  }
}

static void solution1(const char *const input, char *const output) {
  struct blueprint *blueprints;
  int len = parse_input(input, &blueprints);
  for (int i = 0; i < len; i++) {
    preprocess(blueprints + i);
  }

  DBG("ore robot, ore: %d ", blueprints[1].ore_ore);
  DBG("clay robot, ore: %d ", blueprints[1].clay_ore);
  DBG("obsidian robot, ore: %d ", blueprints[1].obsidian_ore);
  DBG("obsidian robot, clay: %d ", blueprints[1].obsidian_clay);
  DBG("geode robot, ore: %d ", blueprints[1].geode_ore);
  DBG("geode robot, obsidian: %d ", blueprints[1].geode_obsidian);

  int res = 0;
  for (int i = 0; i < len; i++) {
    struct state init = {
        .ore = 0,
        .clay = 0,
        .obsidian = 0,
        .ore_robots = 1,
        .clay_robots = 0,
        .obsidian_robots = 0,
    };
    res += geods(&blueprints[i], &init, 24, false, false, false, false) * (i + 1);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  free(blueprints);
}

static void solution2(const char *const input, char *const output) {
  struct blueprint *blueprints;
  int len = parse_input(input, &blueprints);
  for (int i = 0; i < len; i++) {
    preprocess(blueprints + i);
  }

  int res = 1;
  for (int i = 0; i < 3; i++) {
    struct state init = {
        .ore = 0,
        .clay = 0,
        .obsidian = 0,
        .ore_robots = 1,
        .clay_robots = 0,
        .obsidian_robots = 0,
    };
    res *= geods(&blueprints[i], &init, 32, false, false, false, false);
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  free(blueprints);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
