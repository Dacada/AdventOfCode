#include <aoclib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define MAX_ANTENNA_NUMS ('9' - '0' + 1)
#define MAX_ANTENNA_UPPER ('Z' - 'A' + 1)
#define MAX_ANTENNA_LOWER ('z' - 'a' + 1)
#define MAX_ANTENNA (MAX_ANTENNA_NUMS + MAX_ANTENNA_UPPER + MAX_ANTENNA_LOWER)

struct point {
  int x;
  int y;
};

__attribute__((pure)) static int index_antenna(char name) {
  if (name >= '0' && name <= '9') {
    return name - '0';
  } else if (name >= 'A' && name <= 'Z') {
    return name - 'A' + MAX_ANTENNA_NUMS;
  } else if (name >= 'a' && name <= 'z') {
    return name - 'a' + MAX_ANTENNA_UPPER + MAX_ANTENNA_NUMS;
  }
  FAIL("invalid antenna name '%c'", name);
}

#ifdef DEBUG
__attribute__((pure)) static int name_antenna(int index) {
  if (index >= 0 && index < MAX_ANTENNA_NUMS) {
    return index + '0';
  } else if (index < MAX_ANTENNA_UPPER + MAX_ANTENNA_NUMS) {
    return index - MAX_ANTENNA_NUMS + 'A';
  } else if (index < MAX_ANTENNA) {
    return index - MAX_ANTENNA_UPPER - MAX_ANTENNA_NUMS + 'a';
  }
  FAIL("invalid antenna index %d", index);
}

static void display_output(struct point max, struct aoc_dynarr *antenna_collections, bool *has_antipode) {
  char *buff = malloc(sizeof(*buff) * max.x * max.y);
  for (int i = 0; i < max.x * max.y; i++) {
    buff[i] = '.';
  }
  for (int i = 0; i < MAX_ANTENNA; i++) {
    struct aoc_dynarr antenna_collection = antenna_collections[i];
    if (antenna_collection.size != 0) {
      char c = name_antenna(i);
      struct point *antennas = antenna_collection.data;
      for (int j = 0; j < antenna_collection.len; j++) {
        struct point p = antennas[j];
        buff[p.y * max.x + p.x] = c;
      }
    }
  }
  for (int j = 0; j < max.y; j++) {
    for (int i = 0; i < max.x; i++) {
      if (has_antipode[j * max.x + i]) {
        fputc('#', stderr);
      } else {
        fputc(buff[j * max.x + i], stderr);
      }
    }
    fputc('\n', stderr);
  }
  free(buff);
}
#else
static void display_output(struct point max, struct aoc_dynarr *antenna_collections, bool *has_antipode) {
  (void)max;
  (void)antenna_collections;
  (void)has_antipode;
}
#endif

static void parse_input(const char *input, struct aoc_dynarr *antennas, struct point *max) {
  max->x = 0;
  max->y = 0;
  for (int y = 0;; y++) {
    for (int x = 0;; x++) {
      if (*input == '\0') {
        break;
      }

      if (*input == '\n') {
        max->x = x;
        input++;
        break;
      }

      if (*input == '.') {
        input++;
        continue;
      }

      int i = index_antenna(*input);
      if (antennas[i].size == 0) {
        aoc_dynarr_init(&antennas[i], sizeof(struct point), 2);
      }
      struct point *p = aoc_dynarr_grow(&antennas[i], 1);
      p->x = x;
      p->y = y;
      input++;
    }
    if (*input == '\0') {
      max->y = y;
      break;
    }
  }
  max->y += 1;
}

static bool is_point_valid(struct point p, struct point max) {
  return p.x >= 0 && p.x < max.x && p.y >= 0 && p.y < max.y;
}

static void find_one_antipode(struct point ant1, struct point ant2, struct point *anti) {
  anti->x = ant1.x + (ant1.x - ant2.x);
  anti->y = ant1.y + (ant1.y - ant2.y);
}

static void find_all_antipodes(struct point ant1, struct point ant2, struct aoc_dynarr *antis, struct point max) {
  struct point d;
  d.x = ant1.x - ant2.x;
  d.y = ant1.y - ant2.y;

  for (int i = 0;; i++) {
    struct point anti = ant1;
    anti.x += d.x * i;
    anti.y += d.y * i;

    if (!is_point_valid(anti, max)) {
      break;
    }

    struct point *res = aoc_dynarr_grow(antis, 1);
    *res = anti;
  }
}

static void solution(const char *input, char *const output, bool find_all) {
  static struct aoc_dynarr antenna_collections[MAX_ANTENNA];
  struct point max;
  parse_input(input, antenna_collections, &max);

  bool *has_antipode = malloc(max.x * max.y * sizeof(*has_antipode));
  memset(has_antipode, 0, max.x * max.y * sizeof(*has_antipode));
  int total = 0;

  struct aoc_dynarr antipodes;
  if (find_all) {
    aoc_dynarr_init(&antipodes, sizeof(struct point), 4);
  }

  for (int i = 0; i < MAX_ANTENNA; i++) {
    struct aoc_dynarr antenna_collection = antenna_collections[i];
    if (antenna_collection.size == 0) {
      continue;
    }
    struct point *antennas = antenna_collection.data;
    for (int j = 0; j < antenna_collection.len; j++) {
      for (int k = 0; k < antenna_collection.len; k++) {
        if (j == k) {
          continue;
        }
        struct point ant1 = antennas[j];
        struct point ant2 = antennas[k];

        if (find_all) {
          aoc_dynarr_truncate(&antipodes);
          find_all_antipodes(ant1, ant2, &antipodes, max);
          for (int l = 0; l < antipodes.len; l++) {
            struct point *antis = antipodes.data;
            struct point anti = antis[l];
            if (!has_antipode[anti.y * max.x + anti.x]) {
              has_antipode[anti.y * max.x + anti.x] = true;
              total++;
            }
          }
        } else {
          struct point anti;
          find_one_antipode(ant1, ant2, &anti);
          if (is_point_valid(anti, max) && !has_antipode[anti.y * max.x + anti.x]) {
            has_antipode[anti.y * max.x + anti.x] = true;
            total++;
          }
        }
      }
    }
  }

  if (find_all) {
    aoc_dynarr_free(&antipodes);
  }

  display_output(max, antenna_collections, has_antipode);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
  free(has_antipode);
  for (int i = 0; i < MAX_ANTENNA; i++) {
    if (antenna_collections[i].size != 0) {
      aoc_dynarr_free(&antenna_collections[i]);
    }
  }
}

static void solution1(const char *input, char *const output) { solution(input, output, false); }

static void solution2(const char *input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
