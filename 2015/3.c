#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct pair_t {
  int16_t item1;
  int16_t item2;
} pair_t;

static int pair_cmp(const void *e1, const void *e2) {
  const pair_t *pair1 = e1;
  const pair_t *pair2 = e2;

  uint32_t n1 = (((uint16_t)pair1->item1) << 16) | ((uint16_t)pair1->item2);
  uint32_t n2 = (((uint16_t)pair2->item1) << 16) | ((uint16_t)pair2->item2);

  if (n1 > n2) return 1;
  else if (n1 < n2) return -1;
  else return 0;
}

static int get_count(pair_t *const elements) {
  qsort(elements, 8192, sizeof(pair_t), pair_cmp);

  bool found_origin = false;
  pair_t last_pair = elements[0];
  if (last_pair.item1 == 0 && last_pair.item2 == 0) {
    found_origin = true;
  }
  int count = 2; // one for the origin, another for the first element
  for (size_t i=1; i<8192; i++) {
    if (last_pair.item1 != elements[i].item1 || last_pair.item2 != elements[i].item2) {
      count++;
      last_pair = elements[i];
      
      if (last_pair.item1 == 0 && last_pair.item2 == 0) {
	fprintf(stderr, "%d %d\n", last_pair.item1, last_pair.item2);
	found_origin = true;
      }
    }
  }

  if (found_origin) {
    count--; // if it finds the origin, it'll count it twice: once implicit at the start and twice at the middle
  }

  return count;
}

static void solution1(const char *const input, char *const output) {
  pair_t elements[8192];
  memset(elements, 0, sizeof(elements));

  int16_t x = 0;
  int16_t y = 0;
  for (size_t i=0; input[i] != '\0'; i++) {
    char c = input[i];
    switch (c) {
    case '^':
      y -= 1; break;
    case 'v':
      y += 1; break;
    case '<':
      x -= 1; break;
    case '>':
      x += 1; break;
    }
    elements[i].item1 = x;
    elements[i].item2 = y;
  }

  const int count = get_count(elements);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static void solution2(const char *const input, char *const output) {
  pair_t elements[8192];
  memset(elements, 0, sizeof(elements));

  int16_t x = 0, robox = 0;
  int16_t y = 0, roboy = 0;
  bool santa = true;
  
  for (size_t i=0; input[i] != '\0'; i++) {
    const char c = input[i];
    
    switch (c) {
    case '^':
      if (santa) y -= 1;
      else roboy -= 1;
      break;
    case 'v':
      if (santa) y += 1;
      else roboy += 1;
      break;
    case '<':
      if (santa) x -= 1;
      else robox -= 1;
      break;
    case '>':
      if (santa) x += 1;
      else robox += 1;
      break;
    }
    
    if (santa) {
      elements[i].item1 = x;
      elements[i].item2 = y;
    } else {
      elements[i].item1 = robox;
      elements[i].item2 = roboy;
    }

    santa = !santa;
  }

  const int count = get_count(elements);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
