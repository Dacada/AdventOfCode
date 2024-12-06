#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define MAX_NUM_VALVES (26 * 26)
#define IDX(x, y, len) ((y) * (len) + (x))

// given a valve, e.g. AA, return its idx, e.g. 12
static int valve_name_map[MAX_NUM_VALVES];

// given an idx, e.g. 12 return a valve, e.g. AA
static int valve_name_map_reverse[MAX_NUM_VALVES];

static void skip_newlines(const char **input) {
  while (**input == '\n') {
    *input += 1;
  }
}

static void init_valve_name_map(void) {
  for (int i = 0; i < MAX_NUM_VALVES; i++) {
    valve_name_map[i] = -1;
    valve_name_map_reverse[i] = -1;
  }
}

static int parse_valve(const char **input) {
  char c1 = **input;
  *input += 1;
  char c2 = **input;
  *input += 1;
  ASSERT(isupper(c1) && isupper(c2), "parse error");

  int a = c1 - 'A';
  int b = c2 - 'A';
  return a * 26 + b;
}

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

static void parse_line_1(const char **input, int i) {
  ASSERT_STR(*input, "Valve ");
  int v = parse_valve(input);
  valve_name_map[v] = i;
  valve_name_map_reverse[i] = v;
  while (**input != '\n') {
    *input += 1;
  }
}

static void parse_line_2(const char **input, int *flow_rates, bool *tunnels, int len) {
  ASSERT_STR(*input, "Valve ");
  int v = parse_valve(input);
  int i = valve_name_map[v];
  ASSERT(i >= 0, "parse error");

  ASSERT_STR(*input, " has flow rate=");
  flow_rates[i] = parse_int(input);

  ASSERT_STR(*input, "; tunnel");
  if (**input == 's') {
    *input += 1;
  }
  ASSERT_STR(*input, " lead");
  if (**input == 's') {
    *input += 1;
  }
  ASSERT_STR(*input, " to valve");
  if (**input == 's') {
    *input += 1;
  }
  ASSERT_STR(*input, " ");

  while (**input != '\n') {
    v = parse_valve(input);
    int j = valve_name_map[v];
    ASSERT(j >= 0, "parse error");
    tunnels[IDX(i, j, len)] = true;
    // tunnels[IDX(j,i,len)] = true; // not needed since when we check j's line it should have a tunnel to i
    if (**input == ',') {
      *input += 1;
      ASSERT(**input == ' ', "parse error");
      *input += 1;
    } else {
      ASSERT(**input == '\n', "parse error");
    }
  }
}

#undef ASSERT_STR

static int parse_input(const char *input, int **flow_rates, bool **tunnels) {
  const char *orig = input;
  init_valve_name_map();

  int len = 0;
  while (*input != '\0') {
    parse_line_1(&input, len++);
    skip_newlines(&input);
  }

  *flow_rates = malloc(sizeof(**flow_rates) * len);
  *tunnels = malloc(sizeof(**tunnels) * len * len);
  memset(*tunnels, 0, sizeof(**tunnels) * len * len);

  input = orig;
  while (*input != '\0') {
    parse_line_2(&input, *flow_rates, *tunnels, len);
    skip_newlines(&input);
  }

  return len;
}

static void calc_valve_distances(const bool *tunnels, int **distances, int len) {
  int *dist = *distances = malloc(sizeof(**distances) * len * len);
  for (int i = 0; i < len * len; i++) {
    dist[i] = INT_MAX;
  }
  for (int i = 0; i < len; i++) {
    for (int j = 0; j < len; j++) {
      if (tunnels[IDX(i, j, len)]) {
        dist[IDX(i, j, len)] = 1;
      }
    }
  }
  for (int i = 0; i < len; i++) {
    dist[IDX(i, i, len)] = 0;
  }
  for (int k = 0; k < len; k++) {
    for (int i = 0; i < len; i++) {
      int a = dist[IDX(i, k, len)];
      if (a == INT_MAX) {
        continue;
      }
      for (int j = 0; j < len; j++) {
        int b = dist[IDX(k, j, len)];
        if (b == INT_MAX) {
          continue;
        }

        int d = a + b;
        if (dist[IDX(i, j, len)] > d) {
          dist[IDX(i, j, len)] = d;
        }
      }
    }
  }
}

static void remove_useless_valves(int **distances, int **distances_for_aa, int **flow_rates, int *length) {
  int old_len = *length;
  int *new_flow_rates = malloc(sizeof(*new_flow_rates) * old_len);
  *distances_for_aa = malloc(sizeof(**distances_for_aa) * old_len);
  static int new_valve_map_reverse[MAX_NUM_VALVES];

  int new_len = 0;
  for (int i = 0; i < old_len; i++) {
    int f = (*flow_rates)[i];
    if (f == 0) {
      continue;
    }

    int v = new_valve_map_reverse[new_len] = valve_name_map_reverse[i];
    valve_name_map[v] = new_len;

    new_flow_rates[new_len] = f;
    (*distances_for_aa)[new_len] = (*distances)[IDX(i, valve_name_map[0], old_len)];
    new_len++;
  }
  new_flow_rates = realloc(new_flow_rates, sizeof(*new_flow_rates) * new_len);
  *distances_for_aa = realloc(*distances_for_aa, sizeof(**distances_for_aa) * new_len);
  memcpy(valve_name_map_reverse, new_valve_map_reverse, sizeof(valve_name_map_reverse));

  int *new_distances = malloc(sizeof(*new_distances) * new_len * new_len);
  int ii = 0;
  for (int i = 0; i < old_len; i++) {
    if ((*flow_rates)[i] == 0) {
      continue;
    }
    int jj = 0;
    for (int j = 0; j < old_len; j++) {
      if ((*flow_rates)[j] == 0) {
        continue;
      }
      new_distances[IDX(ii, jj, new_len)] = (*distances)[IDX(i, j, old_len)];
      jj++;
    }
    ii++;
  }

  free(*distances);
  free(*flow_rates);
  *distances = new_distances;
  *flow_rates = new_flow_rates;
  *length = new_len;
}

static int best_path(int len, const int *flow_rates, const int *distances, int minutes, int current,
                     const bool *open_valves) {
  int best = 0;
  for (int i = 0; i < len; i++) {
    if (open_valves[i]) {
      continue;
    }

    int minutes_to_travel = distances[IDX(current, i, len)];
    if (minutes_to_travel >= minutes) {
      continue;
    }

    int minutes_left = minutes - minutes_to_travel - 1;
    int released_pressure = minutes_left * flow_rates[i];

    bool *new_open_valves = malloc(sizeof(*new_open_valves) * len);
    memcpy(new_open_valves, open_valves, sizeof(*new_open_valves) * len);
    new_open_valves[i] = true;
    int pressure = released_pressure + best_path(len, flow_rates, distances, minutes_left, i, new_open_valves);
    free(new_open_valves);

    if (pressure > best) {
      best = pressure;
    }
  }
  return best;
}

static int best_path_2(int len, const int *flow_rates, const int *distances, const int *starting_distances, int minutes,
                       int current, const bool *open_valves, bool restarted) {
  int best = 0;
  for (int i = 0; i < len; i++) {
    if (open_valves[i]) {
      continue;
    }

    int minutes_to_travel = distances[IDX(current, i, len)];
    if (minutes_to_travel >= minutes) {
      continue;
    }

    int minutes_left = minutes - minutes_to_travel - 1;
    int released_pressure = minutes_left * flow_rates[i];

    bool *new_open_valves = malloc(sizeof(*new_open_valves) * len);
    memcpy(new_open_valves, open_valves, sizeof(*new_open_valves) * len);
    new_open_valves[i] = true;

    int pressure = released_pressure + best_path_2(len, flow_rates, distances, starting_distances, minutes_left, i,
                                                   new_open_valves, restarted);
    free(new_open_valves);

    if (pressure > best) {
      best = pressure;
    }
  }

  if (!restarted) {
    for (int i = 0; i < len; i++) {
      if (open_valves[i]) {
        continue;
      }

      int minutes_to_travel = starting_distances[i];
      int minutes_left = 26 - minutes_to_travel - 1;
      int released_pressure = minutes_left * flow_rates[i];
      bool *new_open_valves = malloc(sizeof(*new_open_valves) * len);
      memcpy(new_open_valves, open_valves, sizeof(*new_open_valves) * len);
      new_open_valves[i] = true;
      int pressure = released_pressure + best_path(len, flow_rates, distances, minutes_left, i, new_open_valves);
      free(new_open_valves);
      if (pressure > best) {
        best = pressure;
      }
    }
  }

  return best;
}

static int best_ordering_pressure(const int *flow_rates, const int *distances, const int *starting_distances, int len) {
  int best = 0;
  for (int i = 0; i < len; i++) {
    int minutes_to_travel = starting_distances[i];
    int minutes_left = 30 - minutes_to_travel - 1;
    int released_pressure = minutes_left * flow_rates[i];
    bool *open_valves = malloc(sizeof(*open_valves) * len);
    memset(open_valves, 0, sizeof(*open_valves) * len);
    open_valves[i] = true;
    int pressure = released_pressure + best_path(len, flow_rates, distances, minutes_left, i, open_valves);
    free(open_valves);
    if (pressure > best) {
      best = pressure;
    }
  }
  return best;
}

static int best_ordering_pressure_2(const int *flow_rates, const int *distances, const int *starting_distances,
                                    int len) {
  int best = 0;
  for (int i = 0; i < len; i++) {
    int minutes_to_travel = starting_distances[i];
    int minutes_left = 26 - minutes_to_travel - 1;
    int released_pressure = minutes_left * flow_rates[i];
    bool *open_valves = malloc(sizeof(*open_valves) * len);
    memset(open_valves, 0, sizeof(*open_valves) * len);
    open_valves[i] = true;
    int pressure = released_pressure +
                   best_path_2(len, flow_rates, distances, starting_distances, minutes_left, i, open_valves, false);
    free(open_valves);
    if (pressure > best) {
      best = pressure;
    }
  }
  return best;
}

static void solution(const char *const input, char *const output,
                     int (*calc_pressure)(const int *, const int *, const int *, int)) {
  int *flow_rates;
  bool *tunnels;
  int len = parse_input(input, &flow_rates, &tunnels);

  int *distances;
  calc_valve_distances(tunnels, &distances, len);
  free(tunnels);

  int *distances_aa;
  remove_useless_valves(&distances, &distances_aa, &flow_rates, &len);

#ifdef DEBUG
  for (int i = 0; i < len; i++) {
    int v = valve_name_map_reverse[i];
    fprintf(stderr, "Valve %c%c has flow rate %d\n", v / 26 + 'A', v % 26 + 'A', flow_rates[i]);
  }
  fputc('\n', stderr);
  for (int i = 0; i < len; i++) {
    int v = valve_name_map_reverse[i];
    fprintf(stderr, "Valve AA is at distance %d from valve %c%c\n", distances_aa[i], v / 26 + 'A', v % 26 + 'A');
  }
  for (int i = 0; i < len; i++) {
    int vi = valve_name_map_reverse[i];
    for (int j = 0; j < len; j++) {
      int vj = valve_name_map_reverse[j];
      fprintf(stderr, "Valve %c%c is at distance %d from valve %c%c\n", vi / 26 + 'A', vi % 26 + 'A',
              distances[IDX(i, j, len)], vj / 26 + 'A', vj % 26 + 'A');
    }
  }
#endif

  int res = calc_pressure(flow_rates, distances, distances_aa, len);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  free(flow_rates);
  free(distances);
  free(distances_aa);
}

static void solution1(const char *const input, char *const output) { solution(input, output, best_ordering_pressure); }

static void solution2(const char *const input, char *const output) {
  solution(input, output, best_ordering_pressure_2);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
