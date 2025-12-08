#include <aoclib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_CONNECTIONS 1000

struct point3d {
  long x, y, z;
};

struct distance {
  int pa, pb;
  long dist;
};

static int distance_cmp(const void *a, const void *b) {
  const struct distance *da = a;
  const struct distance *db = b;

  if (da->dist < db->dist) {
    return -1;
  }
  if (da->dist > db->dist) {
    return 1;
  }
  return 0;
}

static bool parse_point3d(const char **input, void *ptr) {
  struct point3d *point = ptr;
  if (**input == '\0') {
    return false;
  }
  point->x = aoc_parse_long(input);
  aoc_expect_char(input, ',');
  point->y = aoc_parse_long(input);
  aoc_expect_char(input, ',');
  point->z = aoc_parse_long(input);
  if (**input == '\n') {
    *input += 1;
  } else {
    ASSERT(**input == '\0', "parse error");
  }
  return true;
}

static long compute_distance(int i, int j, const struct point3d *points) {
  struct point3d a = points[i];
  struct point3d b = points[j];
  struct point3d p;
  p.x = a.x - b.x;
  p.y = a.y - b.y;
  p.z = a.z - b.z;
  return p.x * p.x + p.y * p.y + p.z * p.z;
}

static struct distance *compute_all_distances(const struct point3d *points, int npoints, int *ndistances) {
  *ndistances = npoints * (npoints - 1) / 2;
  struct distance *distances = malloc(sizeof(*distances) * *ndistances);

  int k = 0;
  for (int i = 0; i < npoints; i++) {
    for (int j = i + 1; j < npoints; j++) {
      struct distance dist;
      dist.pa = i;
      dist.pb = j;
      dist.dist = compute_distance(i, j, points);
      distances[k++] = dist;
    }
  }

  return distances;
}

static bool connect_to_circuit(int pa, int pb, int *circuits, int *next_circuit, int npoints) {
  int c1 = circuits[pa];
  int c2 = circuits[pb];

  if (c1 == 0 && c2 == 0) {
    // neither is assigned to a circuit, create a new circuit with them both
    circuits[pa] = *next_circuit;
    circuits[pb] = *next_circuit;
    *next_circuit += 1;
  } else if (c1 == 0 || c2 == 0) {
    // one of them is in a circuit and the other isn't, add the new one to the circuit
    int c, d;
    if (c1 == 0) {
      c = c2;
      d = pa;
    } else {
      c = c1;
      d = pb;
    }
    circuits[d] = c;
  } else if (c1 != c2) {
    // both of them are already in a different circuit, merge the circuits together
    for (int i = 0; i < npoints; i++) {
      if (circuits[i] == c2) {
        circuits[i] = c1;
      }
    }
  }

  int n = circuits[0];
  for (int i = 1; i < npoints; i++) {
    if (circuits[i] != n) {
      return false;
    }
  }

  return true;
}

static void solution1(const char *input, char *const output) {
  int npoints;
  struct point3d *points = aoc_parse_sequence(&input, &npoints, sizeof(*points), 64, parse_point3d);

  int ndistances;
  struct distance *distances = compute_all_distances(points, npoints, &ndistances);

  qsort(distances, ndistances, sizeof(*distances), distance_cmp);

  int *circuits = malloc(sizeof(*circuits) * npoints);
  memset(circuits, 0, sizeof(*circuits) * npoints);

  int next_circuit = 1;
  for (int i = 0; i < NUM_CONNECTIONS; i++) {
    struct distance dist = distances[i];
    connect_to_circuit(dist.pa, dist.pb, circuits, &next_circuit, npoints);
  }

  int *circuit_sizes = malloc(sizeof(*circuit_sizes) * next_circuit);
  memset(circuit_sizes, 0, sizeof(*circuit_sizes) * next_circuit);
  for (int i = 0; i < npoints; i++) {
    circuit_sizes[circuits[i]] += 1;
  }

  qsort(circuit_sizes, next_circuit, sizeof(*circuit_sizes), aoc_cmp_int);
  int result = 1;
  for (int i = 0; i < 3; i++) {
    int idx = next_circuit - i - 2;
    result *= circuit_sizes[idx];
  }

  free(points);
  free(distances);
  free(circuits);
  free(circuit_sizes);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *input, char *const output) {
  int npoints;
  struct point3d *points = aoc_parse_sequence(&input, &npoints, sizeof(*points), 64, parse_point3d);

  int ndistances;
  struct distance *distances = compute_all_distances(points, npoints, &ndistances);

  qsort(distances, ndistances, sizeof(*distances), distance_cmp);

  int *circuits = malloc(sizeof(*circuits) * npoints);
  memset(circuits, 0, sizeof(*circuits) * npoints);

  int next_circuit = 1;
  bool done = false;
  int i = 0;
  while (!done) {
    struct distance dist = distances[i++];
    done = connect_to_circuit(dist.pa, dist.pb, circuits, &next_circuit, npoints);
  }
  i -= 1;

  long result = points[distances[i].pa].x * points[distances[i].pb].x;

  free(points);
  free(distances);
  free(circuits);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
