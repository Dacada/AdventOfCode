#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

enum color {
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
};

struct game_set {
  int red;
  int green;
  int blue;
};

struct game {
  int num_sets;
  int cap_sets;
  struct game_set *sets;
};

static void assert_str(const char **input, const char *expected) {
  int n = strlen(expected);
  if (strncmp(*input, expected, n) != 0) {
    FAIL("parse error, expected '%s' got '%s' n=%d", expected, *input, n);
  }
  *input += n;
}

static enum color parse_color(const char **input) {
  switch (**input) {
  case 'r':
    assert_str(input, "red");
    return COLOR_RED;
  case 'g':
    assert_str(input, "green");
    return COLOR_GREEN;
  case 'b':
    assert_str(input, "blue");
    return COLOR_BLUE;
  }
  FAIL("parse error");
}

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error");

  int n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }

  return n;
}

static void parse_set(const char **input, struct game_set *set) {
  set->red = 0;
  set->green = 0;
  set->blue = 0;

  for (;;) {
    int n = parse_int(input);
    ASSERT(**input == ' ', "parse error");
    *input += 1;

    switch (parse_color(input)) {
    case COLOR_RED:
      set->red = n;
      break;
    case COLOR_GREEN:
      set->green = n;
      break;
    case COLOR_BLUE:
      set->blue = n;
      break;
    default:
      FAIL("parse error");
    }

    if (**input == '\n' || **input == '\0' || **input == ';') {
      return;
    }

    assert_str(input, ", ");
  }
}

static void parse_line(const char **input, struct game *game) {
  game->num_sets = 0;
  game->cap_sets = 4;
  game->sets = malloc(sizeof(*game->sets) * game->cap_sets);

  assert_str(input, "Game ");
  ASSERT(isdigit(**input), "parse error");
  while (isdigit(**input)) {
    *input += 1;
  }
  assert_str(input, ": ");

  for (;;) {
    if (game->num_sets >= game->cap_sets) {
      game->cap_sets *= 2;
      game->sets = realloc(game->sets, sizeof(*game->sets) * game->cap_sets);
    }
    parse_set(input, game->sets + game->num_sets);
    game->num_sets++;
    if (**input == '\n' || **input == '\0') {
      break;
    }
    assert_str(input, "; ");
  }

  if (**input == '\n') {
    *input += 1;
  }
}

static int parse_input(const char *input, struct game **games) {
  int games_len = 0;
  int games_cap = 16;
  *games = malloc(sizeof(**games) * games_cap);

  while (*input != '\0') {
    if (games_len >= games_cap) {
      games_cap *= 2;
      *games = realloc(*games, sizeof(**games) * games_cap);
    }

    parse_line(&input, *games + games_len);
    games_len++;
  }

  return games_len;
}

static bool is_game_possible(const struct game *game, int red, int green, int blue) {
  for (int i = 0; i < game->num_sets; i++) {
    if (game->sets[i].red > red || game->sets[i].green > green || game->sets[i].blue > blue) {
      return false;
    }
  }
  return true;
}

static void get_minimal_set(const struct game *game, struct game_set *set) {
  set->red = 0;
  set->green = 0;
  set->blue = 0;

  for (int i = 0; i < game->num_sets; i++) {
    int r = game->sets[i].red;
    int g = game->sets[i].green;
    int b = game->sets[i].blue;

    if (r > set->red) {
      set->red = r;
    }
    if (g > set->green) {
      set->green = g;
    }
    if (b > set->blue) {
      set->blue = b;
    }
  }
}

static void solution1(const char *const input, char *const output) {
  struct game *games;
  int num_games = parse_input(input, &games);

#ifdef DEBUG
  for (int i = 0; i < num_games; i++) {
    fprintf(stderr, "Game %d: ", i + 1);
    for (int j = 0; j < games[i].num_sets; j++) {
      int r = games[i].sets[j].red;
      int g = games[i].sets[j].green;
      int b = games[i].sets[j].blue;

      if (r) {
        fprintf(stderr, "%d red", r);
      }
      if (g) {
        if (r) {
          fprintf(stderr, ", ");
        }
        fprintf(stderr, "%d green", g);
      }
      if (b) {
        if (g || r) {
          fprintf(stderr, ", ");
        }
        fprintf(stderr, "%d blue", b);
      }
      fprintf(stderr, "; ");
    }
    fprintf(stderr, "\n");
  }
#endif

  int result = 0;
  for (int i = 0; i < num_games; i++) {
    if (is_game_possible(games + i, 12, 13, 14)) {
      result += i + 1;
    }
  }

  for (int i = 0; i < num_games; i++) {
    free(games[i].sets);
  }
  free(games);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *const input, char *const output) {
  struct game *games;
  int num_games = parse_input(input, &games);

  int result = 0;
  for (int i = 0; i < num_games; i++) {
    struct game_set set;
    get_minimal_set(games + i, &set);
    result += set.red * set.green * set.blue;
  }

  for (int i = 0; i < num_games; i++) {
    free(games[i].sets);
  }
  free(games);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
