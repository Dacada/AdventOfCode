#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

struct vector {
  int x, y, z;
};

static void vector_zero(struct vector *const v) { v->x = v->y = v->z = 0; }

static int vector_cmp(const void *a, const void *b) {
  const struct vector *v1 = a;
  const struct vector *v2 = b;

  if (v1->x == v2->x) {
    if (v1->y == v2->y) {
      if (v1->z == v2->z) {
        return 0;
      } else {
        return v1->z - v2->z;
      }
    } else {
      return v1->y - v2->y;
    }
  } else {
    return v1->x - v2->x;
  }
}

static void vector_add(struct vector *const r, const struct vector *const a, const struct vector *const b) {
  r->x = a->x + b->x;
  r->y = a->y + b->y;
  r->z = a->z + b->z;
}

static void vector_sub(struct vector *const r, const struct vector *const a, const struct vector *const b) {
  r->x = a->x - b->x;
  r->y = a->y - b->y;
  r->z = a->z - b->z;
}

static void position_identity(struct vector *const p) { vector_zero(p); }

/* static void position_inverse(const struct vector *const pfrom, struct vector *const pto) { */
/*         pto->x = -pfrom->x; */
/*         pto->y = -pfrom->y; */
/*         pto->z = -pfrom->z; */
/* } */

// rotation records the front direction (1=x, 2=y, 3=z, 4=-z, 5=-y, 6=-x,
// similar to a dice) and the up direction (same) abs(front) = abs(up) is
// illegal

struct rotation {
  unsigned front;
  unsigned up;
};

static void rotation_identity(struct rotation *const r) {
  r->front = 1;
  r->up = 2;
}

__attribute__((pure)) static unsigned dice_reverse(const unsigned d) {
  switch (d) {
  case 1:
    return 6;
  case 2:
    return 5;
  case 3:
    return 4;
  case 4:
    return 3;
  case 5:
    return 2;
  case 6:
    return 1;
  default:
    FAIL("invalid dice");
  }
}

__attribute__((pure)) static unsigned dice_getZ(const unsigned front, const unsigned up) {
  switch (front) {
  case 1:
    switch (up) {
    case 2:
      return 3;
    case 3:
      return 5;
    case 4:
      return 2;
    case 5:
      return 4;
    default:
      FAIL("invalid dice");
    }
  case 2:
    switch (up) {
    case 1:
      return 4;
    case 3:
      return 1;
    case 4:
      return 6;
    case 6:
      return 3;
    default:
      FAIL("invalid dice");
    }
  case 3:
    switch (up) {
    case 1:
      return 2;
    case 2:
      return 6;
    case 5:
      return 1;
    case 6:
      return 5;
    default:
      FAIL("invalid dice");
    }
  case 4:
    switch (up) {
    case 1:
      return 5;
    case 2:
      return 1;
    case 5:
      return 6;
    case 6:
      return 2;
    default:
      FAIL("invalid dice");
    }
  case 5:
    switch (up) {
    case 1:
      return 3;
    case 3:
      return 6;
    case 4:
      return 1;
    case 6:
      return 4;
    default:
      FAIL("invalid dice");
    }
  case 6:
    switch (up) {
    case 2:
      return 4;
    case 3:
      return 2;
    case 4:
      return 5;
    case 5:
      return 3;
    default:
      FAIL("invalid dice");
    }
  default:
    FAIL("invalid dice");
  }
}

static int dice_getCoord(const unsigned d, const struct vector *const v) {
  switch (d) {
  case 1:
    return v->x;
  case 2:
    return v->y;
  case 3:
    return v->z;
  case 4:
    return -v->z;
  case 5:
    return -v->y;
  case 6:
    return -v->x;
  default:
    FAIL("invalid rotation");
  }
}

/* static void rotation_inverse(const struct rotation *const rfrom, struct rotation *const rto) { */
/*         rto->front = dice_reverse(rfrom->front); */
/*         rto->up = dice_reverse(rfrom->up); */
/* } */

static void rotation_apply_relative(struct rotation *const res, const struct rotation *const from,
                                    const struct rotation *const rel) {
  switch (rel->front) {
  case 1:
    res->front = from->front;
    break;
  case 2:
    res->front = from->up;
    break;
  case 3:
    res->front = dice_getZ(from->front, from->up);
    break;
  case 4:
    res->front = dice_reverse(dice_getZ(from->front, from->up));
    break;
  case 5:
    res->front = dice_reverse(from->up);
    break;
  case 6:
    res->front = dice_reverse(from->front);
    break;
  default:
    FAIL("invalid rotation");
  }

  switch (rel->up) {
  case 1:
    res->up = from->front;
    break;
  case 2:
    res->up = from->up;
    break;
  case 3:
    res->up = dice_getZ(from->front, from->up);
    break;
  case 4:
    res->up = dice_reverse(dice_getZ(from->front, from->up));
    break;
  case 5:
    res->up = dice_reverse(from->up);
    break;
  case 6:
    res->up = dice_reverse(from->front);
    break;
  default:
    FAIL("invalid rotation");
  }
}

static void vector_apply_rot(struct vector *const res, const struct vector *const orig,
                             const struct rotation *const rot) {
  res->x = dice_getCoord(rot->front, orig);
  res->y = dice_getCoord(rot->up, orig);
  res->z = dice_getCoord(dice_getZ(rot->front, rot->up), orig);
}

static struct vector transform_vector(const struct vector *const orig, const struct vector *const pos,
                                      const struct rotation *const rot) {
  struct vector res;
  vector_apply_rot(&res, orig, rot);

  struct vector res2;
  vector_add(&res2, &res, pos);

  return res2;
}

struct scanner {
  struct vector *points;
  unsigned *distances;
  size_t nbeacons;
  struct rotation rotation;
  struct vector position;
};

static void assert_msg(const char **const input, const char *const msg) {
  size_t len = strlen(msg);
  ASSERT(strncmp(*input, msg, len) == 0, "parse error '%s' '%s'", msg, *input);
  *input += len;
}

static int parse_number(const char **const input) {
  bool neg = false;
  if (**input == '-') {
    neg = true;
    *input += 1;
  }

  ASSERT(isdigit(**input), "parse error '%s'", *input);

  int n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }

  if (neg) {
    return -n;
  }
  return n;
}

static void parse_beacon(const char **const input, struct vector *const point) {
  point->x = parse_number(input);

  ASSERT(**input == ',', "parse error");
  *input += 1;

  point->y = parse_number(input);

  ASSERT(**input == ',', "parse error");
  *input += 1;

  point->z = parse_number(input);

  ASSERT(**input == '\n', "parse error");
  *input += 1;
}

static void parse_scanner(const char **const input, struct scanner *const scanner) {
  assert_msg(input, "--- scanner ");
  parse_number(input);
  assert_msg(input, " ---\n");

  scanner->nbeacons = 0;
  size_t size = 32;
  scanner->distances = malloc(sizeof(*scanner->distances) * size * size);
  scanner->points = malloc(sizeof(*scanner->points) * size);
  for (;;) {
    if (scanner->nbeacons >= size) {
      size *= 2;
      scanner->distances = realloc(scanner->distances, sizeof(*scanner->distances) * size * size);
      scanner->points = realloc(scanner->points, sizeof(*scanner->points) * size);
    }

    parse_beacon(input, &scanner->points[scanner->nbeacons]);
    scanner->nbeacons += 1;

    if (**input == '\n' || **input == '\0') {
      if (**input == '\n') {
        *input += 1;
      }
      break;
    }
  }
}

static struct scanner *parse_input(const char *input, size_t *len) {
  *len = 0;
  size_t size = 64;
  struct scanner *list = malloc(sizeof(*list) * size);

  while (*input != '\0') {
    if (*len >= size) {
      size *= 2;
      list = realloc(list, sizeof(*list) * size);
    }
    parse_scanner(&input, &list[*len]);
    *len += 1;
  }
  ASSERT(*input == '\0', "parse error");

  return list;
}

static unsigned calculate_distance(const struct vector *const v1, const struct vector *const v2) {
  // no need for square root since we don't want the actual distance,
  // just a number to compare against eachother
  int diffx = v1->x - v2->x;
  int diffy = v1->y - v2->y;
  int diffz = v1->z - v2->z;
  return diffx * diffx + diffy * diffy + diffz * diffz;
}

static void calculate_distances(struct scanner *const scanners, size_t nscanners) {
  for (unsigned i = 0; i < nscanners; i++) {
    size_t nbeacons = scanners[i].nbeacons;
    for (size_t a = 0; a < nbeacons; a++) {
      for (size_t b = a; b < nbeacons; b++) {
        unsigned distance = calculate_distance(&scanners[i].points[a], &scanners[i].points[b]);
        scanners[i].distances[a * nbeacons + b] = scanners[i].distances[b * nbeacons + a] = distance;
      }
    }
  }
}

static size_t find_overlaps(struct scanner *const scan1, struct scanner *const scan2, unsigned **const match1,
                            unsigned **const match2) {
  size_t nbeacons1 = scan1->nbeacons;
  size_t nbeacons2 = scan2->nbeacons;

  size_t size = 16;
  size_t count = 0;

  *match1 = malloc(sizeof(**match1) * size);
  *match2 = malloc(sizeof(**match2) * size);

  struct pair {
    unsigned a, b;
  };
  struct pair *candidates1 = malloc(sizeof(*candidates1) * nbeacons1);
  struct pair *candidates2 = malloc(sizeof(*candidates2) * nbeacons2);

  bool *found_cand1 = malloc(sizeof(*found_cand1) * nbeacons1);
  memset(found_cand1, 0, sizeof(*found_cand1) * nbeacons1);

  bool *done_cand1 = malloc(sizeof(*done_cand1) * nbeacons1);
  bool *done_cand2 = malloc(sizeof(*done_cand2) * nbeacons2);
  memset(done_cand1, 0, sizeof(*done_cand1) * nbeacons1);
  memset(done_cand2, 0, sizeof(*done_cand2) * nbeacons2);

  for (unsigned i = 0; i < nbeacons1; i++) {
    if (done_cand1[i]) {
      continue;
    }
    for (unsigned j = i + 1; j < nbeacons1; j++) {
      if (done_cand1[j]) {
        continue;
      }
      unsigned dist1 = scan1->distances[j * nbeacons1 + i];
      for (unsigned ii = 0; ii < nbeacons2; ii++) {
        if (done_cand2[ii]) {
          continue;
        }
        for (unsigned jj = ii + 1; jj < nbeacons2; jj++) {
          if (done_cand2[jj]) {
            continue;
          }

          unsigned dist2 = scan2->distances[jj * nbeacons2 + ii];
          if (dist1 == dist2) {
            // DBG("a: distance between %u and %u: %u", i, j, dist1);
            // DBG("b: distance between %u and %u: %u", ii, jj, dist2);

            if (count >= size) {
              size *= 2;
              *match1 = realloc(*match1, sizeof(**match1) * size);
              *match2 = realloc(*match2, sizeof(**match2) * size);
            }

            if (found_cand1[i]) {
              (*match1)[count] = i;
              done_cand1[i] = true;
              if (candidates1[i].a == ii || candidates1[i].b == ii) {
                DBG("Assigned %u = %u", i, ii);
                (*match2)[count] = ii;
                done_cand2[ii] = true;
              } else if (candidates1[i].a == jj || candidates1[i].b == jj) {
                DBG("Assigned %u = %u", i, jj);
                (*match2)[count] = jj;
                done_cand2[jj] = true;
              } else {
                DBG("Warning: unmatched candidates");
                goto end;
              }
              count++;
            } else {
              found_cand1[i] = true;
              candidates1[i].a = ii;
              candidates1[i].b = jj;
            }

            if (count >= size) {
              size *= 2;
              *match1 = realloc(*match1, sizeof(**match1) * size);
              *match2 = realloc(*match2, sizeof(**match2) * size);
            }

            if (found_cand1[j]) {
              (*match1)[count] = j;
              done_cand1[j] = true;
              if (candidates1[j].a == ii || candidates1[j].b == ii) {
                DBG("Assigned %u = %u", j, ii);
                (*match2)[count] = ii;
                done_cand2[ii] = true;
              } else if (candidates1[j].a == jj || candidates1[j].b == jj) {
                DBG("Assigned %u = %u", j, jj);
                (*match2)[count] = jj;
                done_cand2[jj] = true;
              } else {
                DBG("Warning: unmatched candidates");
                goto end;
              }
              count++;
            } else {
              found_cand1[j] = true;
              candidates1[j].a = ii;
              candidates1[j].b = jj;
            }
          }
        }
      }
    }
  }

end:
  free(candidates1);
  free(candidates2);
  free(found_cand1);
  free(done_cand1);
  free(done_cand2);

  return count;
}

static void calculate_absolute(const struct scanner *const from, struct scanner *const to,
                               const struct vector *const offset, const struct rotation *const relRot) {
  struct vector tmp;
  vector_apply_rot(&tmp, offset, &from->rotation);
  vector_add(&to->position, &tmp, &from->position);
  rotation_apply_relative(&to->rotation, relRot, &from->rotation);
}

static void calculate_relative(const struct scanner *const abs, const struct scanner *const rel,
                               const unsigned *const matches_abs, const unsigned *const matches_rel,
                               const size_t nmatches, struct rotation *const relRot, struct vector *const relPos) {
  bool firstMatch = true;
  struct rotation matchRot;
  struct vector matchPos;
  for (unsigned i = 0; i < nmatches; i++) {
    unsigned p1 = i;
    unsigned p2 = (i + 1) % nmatches;

    struct vector p1_abs = abs->points[matches_abs[p1]];
    struct vector p1_rel = rel->points[matches_rel[p1]];

    struct vector p2_abs = abs->points[matches_abs[p2]];
    struct vector p2_rel = rel->points[matches_rel[p2]];

    // iterate every possible orientation
    bool match = false;
    struct rotation rot;
    struct vector offset;
    for (unsigned front = 1; front <= 6; front++) {
      rot.front = front;
      for (unsigned up = 1; up <= 6; up++) {
        rot.up = up;

        if ((front == 1 && (up == 1 || up == 6)) || (front == 2 && (up == 2 || up == 5)) ||
            (front == 3 && (up == 3 || up == 4)) || (front == 4 && (up == 3 || up == 4)) ||
            (front == 5 && (up == 2 || up == 5)) || (front == 6 && (up == 1 || up == 6))) {
          continue;
        }

        // DBG("rot: f:%u, u:%u", rot.front, rot.up);

        // apply the orientation to p1_rel and p2_rel
        struct vector p1_rel_rot, p2_rel_rot;
        vector_apply_rot(&p1_rel_rot, &p1_rel, &rot);
        vector_apply_rot(&p2_rel_rot, &p2_rel, &rot);

        // DBG("p1_rel_rot: %d,%d,%d", p1_rel_rot.x, p1_rel_rot.y, p1_rel_rot.z);
        // DBG("p1_abs: %d,%d,%d", p1_abs.x, p1_abs.y, p1_abs.z);

        // DBG("p2_rel_rot: %d,%d,%d", p2_rel_rot.x, p2_rel_rot.y, p2_rel_rot.z);
        // DBG("p2_abs: %d,%d,%d", p2_abs.x, p2_abs.y, p2_abs.z);

        // offset p1_rel_rot to the position of p1_abs
        vector_sub(&offset, &p1_abs, &p1_rel_rot);

        // DBG("offset: %d,%d,%d", offset.x, offset.y, offset.z);

        // apply the same offset to p2_rel
        struct vector p2_rel_rot_off;
        vector_add(&p2_rel_rot_off, &offset, &p2_rel_rot);

        // does it match p2_abs?
        if (vector_cmp(&p2_rel_rot_off, &p2_abs) == 0) {
          match = true;
          break;
        }
      }
      if (match) {
        break;
      }
    }

    ASSERT(match, "missmatched points?");

#ifndef DEBUG
    *relRot = rot;
    *relPos = offset;
    return;
#endif

    // every match should actually match the same offset and rotation

    if (firstMatch) {
      matchRot = rot;
      matchPos = offset;
    } else {
      ASSERT(rot.front == matchRot.front && rot.up == matchRot.up, "missmatched points?");
      ASSERT(vector_cmp(&matchPos, &offset) == 0, "missmatched points?");
    }
  }

  *relRot = matchRot;
  *relPos = matchPos;
}

static struct scanner *locate_scanners(const char *const input, size_t *len) {
  size_t nscanners;
  struct scanner *scanners = parse_input(input, &nscanners);
  calculate_distances(scanners, nscanners);

  // Find pos/rot relative to eachother

  struct rotation *relativeRotations = malloc(nscanners * nscanners * sizeof(*relativeRotations));
  struct vector *relativePositions = malloc(nscanners * nscanners * sizeof(*relativePositions));
  bool *haveRelative = malloc(nscanners * nscanners * sizeof(*haveRelative));
  memset(haveRelative, 0, sizeof(*haveRelative) * nscanners * nscanners);

  for (unsigned i = 0; i < nscanners; i++) {
    for (unsigned j = i + 1; j < nscanners; j++) {
      unsigned *matches_i;
      unsigned *matches_j;
      unsigned nmatches = find_overlaps(&scanners[i], &scanners[j], &matches_i, &matches_j);

      DBG("Beacon %u and %u overlap %u times", i, j, nmatches);
      if (nmatches >= 12) {
        // DBG("Scanner %u and %u overlap", i, j);
        haveRelative[j * nscanners + i] = true;
        haveRelative[i * nscanners + j] = true;
        calculate_relative(&scanners[i], &scanners[j], matches_i, matches_j, nmatches,
                           &relativeRotations[j * nscanners + i], &relativePositions[j * nscanners + i]);
        /* rotation_inverse(&relativeRotations[j*nscanners+i], &relativeRotations[i*nscanners+j]); */
        /* position_inverse(&relativePositions[j*nscanners+i], &relativePositions[i*nscanners+j]); */
        calculate_relative(&scanners[j], &scanners[i], matches_j, matches_i, nmatches,
                           &relativeRotations[i * nscanners + j], &relativePositions[i * nscanners + j]);
      }

      free(matches_i);
      free(matches_j);
    }
  }

  DBG("position of scanner 1 relative to scanner 0: %d,%d,%d", relativePositions[1 * nscanners + 0].x,
      relativePositions[1 * nscanners + 0].y, relativePositions[1 * nscanners + 0].z);
  DBG("rotation of scanner 1 relative to scanner 0: %u,%u", relativeRotations[1 * nscanners + 0].front,
      relativeRotations[1 * nscanners + 0].up);
  DBG("position of scanner 4 relative to scanner 1: %d,%d,%d", relativePositions[4 * nscanners + 1].x,
      relativePositions[4 * nscanners + 1].y, relativePositions[4 * nscanners + 1].z);
  DBG("rotation of scanner 4 relative to scanner 1: %u,%u", relativeRotations[4 * nscanners + 1].front,
      relativeRotations[4 * nscanners + 1].up);
  DBG("position of scanner 2 relative to scanner 4: %d,%d,%d", relativePositions[2 * nscanners + 4].x,
      relativePositions[2 * nscanners + 4].y, relativePositions[2 * nscanners + 4].z);
  DBG("rotation of scanner 2 relative to scanner 4: %u,%u", relativeRotations[2 * nscanners + 4].front,
      relativeRotations[2 * nscanners + 4].up);
  DBG("position of scanner 4 relative to scanner 2: %d,%d,%d", relativePositions[4 * nscanners + 2].x,
      relativePositions[4 * nscanners + 2].y, relativePositions[2 * nscanners + 4].z);
  DBG("rotation of scanner 4 relative to scanner 2: %u,%u", relativeRotations[4 * nscanners + 2].front,
      relativeRotations[4 * nscanners + 2].up);

  // Find pos/rot relative to 0 (absolute)

  rotation_identity(&scanners[0].rotation);
  position_identity(&scanners[0].position);
  bool *hasAbsolute = malloc(nscanners * sizeof(*hasAbsolute));
  memset(hasAbsolute, 0, sizeof(bool) * nscanners);
  hasAbsolute[0] = true;
  bool done = false;
  while (!done) {
    done = true;
    for (unsigned i = 0; i < nscanners; i++) {
      if (!hasAbsolute[i]) {
        done = false;
        continue;
      }
      for (unsigned j = 0; j < nscanners; j++) {
        if (hasAbsolute[j]) {
          continue;
        }

        if (haveRelative[j * nscanners + i]) {
          DBG("Absolte coordinates of scanner %u calculated from scanner %u", j, i);
          calculate_absolute(&scanners[i], &scanners[j], &relativePositions[j * nscanners + i],
                             &relativeRotations[j * nscanners + i]);
          hasAbsolute[j] = true;
        }
      }
    }
  }

  free(hasAbsolute);
  free(haveRelative);
  free(relativePositions);
  free(relativeRotations);

  DBG("absolute position of scanner 0: %d,%d,%d", scanners[0].position.x, scanners[0].position.y,
      scanners[0].position.z);
  DBG("absolute rotation of scanner 0: %u,%u", scanners[0].rotation.front, scanners[0].rotation.up);
  DBG("absolute position of scanner 1: %d,%d,%d", scanners[1].position.x, scanners[1].position.y,
      scanners[1].position.z);
  DBG("absolute rotation of scanner 1: %u,%u", scanners[1].rotation.front, scanners[1].rotation.up);
  DBG("absolute position of scanner 2: %d,%d,%d", scanners[2].position.x, scanners[2].position.y,
      scanners[2].position.z);
  DBG("absolute rotation of scanner 2: %u,%u", scanners[2].rotation.front, scanners[2].rotation.up);
  DBG("absolute position of scanner 3: %d,%d,%d", scanners[3].position.x, scanners[3].position.y,
      scanners[3].position.z);
  DBG("absolute rotation of scanner 3: %u,%u", scanners[3].rotation.front, scanners[3].rotation.up);
  DBG("absolute position of scanner 4: %d,%d,%d", scanners[4].position.x, scanners[4].position.y,
      scanners[4].position.z);
  DBG("absolute rotation of scanner 4: %u,%u", scanners[4].rotation.front, scanners[4].rotation.up);

  *len = nscanners;
  return scanners;
}

static void solution1(const char *const input, char *const output) {
  size_t nscanners;
  struct scanner *scanners = locate_scanners(input, &nscanners);

  // Apply absolute to all beacons

  size_t nbeacons_total = 0;
  for (unsigned i = 0; i < nscanners; i++) {
    nbeacons_total += scanners[i].nbeacons;
  }

  struct vector *beacons = malloc(sizeof(*beacons) * nbeacons_total);
  {
    size_t k = 0;
    for (unsigned i = 0; i < nscanners; i++) {
      size_t nbeacons = scanners[i].nbeacons;
      for (unsigned j = 0; j < nbeacons; j++) {
        beacons[k++] = transform_vector(&scanners[i].points[j], &scanners[i].position, &scanners[i].rotation);
        DBG("Scanner %u, beacon %u: %d,%d,%d", i, j, beacons[k - 1].x, beacons[k - 1].y, beacons[k - 1].z);
      }
    }
  }

  for (unsigned i = 0; i < nscanners; i++) {
    free(scanners[i].distances);
    free(scanners[i].points);
  }

  free(scanners);

  // Sort beacons to easily discard duplicates

  qsort(beacons, nbeacons_total, sizeof(*beacons), vector_cmp);

  // Count unique beacons

  struct vector prev = beacons[0];
  unsigned count = 1;
  for (unsigned i = 1; i < nbeacons_total; i++) {
    struct vector curr = beacons[i];
    if (vector_cmp(&prev, &curr) != 0) {
      count++;
      prev = curr;
    }
  }

  free(beacons);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static unsigned manhattan_distance(const struct vector *const v1, const struct vector *const v2) {
#define abs(x) (((x) < 0) ? (-(x)) : (x))
  return abs(v1->x - v2->x) + abs(v1->y - v2->y) + abs(v1->z - v2->z);
#undef abs
}

static void solution2(const char *const input, char *const output) {
  size_t nscanners;
  struct scanner *scanners = locate_scanners(input, &nscanners);

  unsigned max = 0;
  for (size_t i = 0; i < nscanners; i++) {
    for (size_t j = i + 1; j < nscanners; j++) {
      unsigned distance = manhattan_distance(&scanners[i].position, &scanners[j].position);
      if (distance > max) {
        max = distance;
        DBG("%lu (%d,%d,%d) %lu (%d,%d,%d)", i, scanners[i].position.x, scanners[i].position.y, scanners[i].position.z,
            j, scanners[j].position.x, scanners[j].position.y, scanners[j].position.z);
      }
    }
  }

  for (unsigned i = 0; i < nscanners; i++) {
    free(scanners[i].distances);
    free(scanners[i].points);
  }
  free(scanners);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", max);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
