#include <aoclib.h>
#include <stdio.h>
#include <string.h>

struct schematic {
  int heights[5];
};

static void parse_input_callback(const char **input, void *vres, int x, int y, void *vargs) {
  (void)vres;
  struct schematic *sch = vargs;
  if (y > 0 && y < 6) {
    if (**input == '#') {
      sch->heights[x]++;
    } else {
      ASSERT(**input == '.', "parse error %c", **input);
    }
  }
  *input += 1;
}

static void parse_input(const char *input, struct schematic **keys, int *nkeys, struct schematic **locks, int *nlocks) {
  struct aoc_dynarr key_arr;
  struct aoc_dynarr lock_arr;
  aoc_dynarr_init(&key_arr, sizeof(struct schematic), 8);
  aoc_dynarr_init(&lock_arr, sizeof(struct schematic), 8);

  while (*input != '\0') {
    struct schematic *sch;
    if (*input == '#') {
      sch = aoc_dynarr_grow(&lock_arr, 1);
    } else {
      sch = aoc_dynarr_grow(&key_arr, 1);
    }
    memset(&sch->heights, 0, sizeof(sch->heights));

    int h, w;
    void *x = aoc_parse_grid(&input, parse_input_callback, 1, &h, &w, sch);
    ASSERT(h == 7, "parse error");
    ASSERT(w == 5, "parse error");
    free(x);
    aoc_skip_space(&input);
  }

  *keys = key_arr.data;
  *nkeys = key_arr.len;
  *locks = lock_arr.data;
  *nlocks = lock_arr.len;
}

static bool match(struct schematic *a, struct schematic *b) {
  DBG(" ");
  DBG("key: %d,%d,%d,%d,%d", a->heights[0], a->heights[1], a->heights[2], a->heights[3], a->heights[4]);
  DBG("lock: %d,%d,%d,%d,%d", b->heights[0], b->heights[1], b->heights[2], b->heights[3], b->heights[4]);

  for (int i = 0; i < 5; i++) {
    if (a->heights[i] + b->heights[i] >= 6) {
      DBG("overlap in col %d", i);
      return false;
    }
  }
  DBG("fit");
  return true;
}

static void solution1(const char *input, char *const output) {
  struct schematic *keys;
  struct schematic *locks;
  int nkeys;
  int nlocks;
  parse_input(input, &keys, &nkeys, &locks, &nlocks);

  for (int i = 0; i < nkeys; i++) {
    DBG("key: %d,%d,%d,%d,%d", keys[i].heights[0], keys[i].heights[1], keys[i].heights[2], keys[i].heights[3],
        keys[i].heights[4]);
  }
  for (int i = 0; i < nlocks; i++) {
    DBG("lock: %d,%d,%d,%d,%d", locks[i].heights[0], locks[i].heights[1], locks[i].heights[2], locks[i].heights[3],
        locks[i].heights[4]);
  }

  int count = 0;
  for (int i = 0; i < nkeys; i++) {
    for (int j = 0; j < nlocks; j++) {
      if (match(&keys[i], &locks[j])) {
        count++;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
  free(keys);
  free(locks);
}

static void solution2(const char *input, char *const output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "10 more years");
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
