#include <aoclib.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define TILE_SIDE 10

enum direction {
  UP,
  RIGHT,
  DOWN,
  LEFT,
};

struct tile {
  int id;
  char pixels[TILE_SIDE * TILE_SIDE];
};
inline static char *idxpixel(struct tile *const tile, const int x, const int y) {
  return tile->pixels + y * TILE_SIDE + x;
}
inline static const char *idxpixel_const(const struct tile *const tile, const int x, const int y) {
  return tile->pixels + y * TILE_SIDE + x;
}

enum rotation { ROTATION_NONE, ROTATION_CLOCKWISE, ROTATION_HALF, ROTATION_COUNTERCLOCKWISE, ROTATION_LAST };

enum flip {
  FLIP_NONE,
  FLIP_HORIZONTAL,
  // FLIP_VERTICAL, These are also covered by combinations of rotation and horizontal flips
  // FLIP_BOTH, No need to consdier both flips, as they undo eachother and become just a rotation
  FLIP_LAST
};

struct transform {
  enum rotation rotation;
  enum flip flip;
};

struct image {
  int side;
  const struct tile **tiles;
  struct transform *transforms;
};
static inline const struct tile **idxtile(struct image *const image, const int x, const int y) {
  return image->tiles + y * image->side + x;
}
static inline const struct tile *const *idxtile_const(const struct image *const image, const int x, const int y) {
  return image->tiles + y * image->side + x;
}
static inline struct transform *idxtrans(struct image *const image, const int x, const int y) {
  return image->transforms + y * image->side + x;
}
static inline const struct transform *idxtrans_const(const struct image *const image, const int x, const int y) {
  return image->transforms + y * image->side + x;
}
static void image_init(struct image *const image, int side) {
  image->side = side;

  image->tiles = malloc(side * side * sizeof *image->tiles);
  image->transforms = malloc(side * side * sizeof *image->transforms);

  for (int j = 0; j < side; j++) {
    for (int i = 0; i < side; i++) {
      *idxtile(image, i, j) = NULL;
    }
  }
}
static void image_free(struct image *const image) {
  free(image->tiles);
  free(image->transforms);
}
static void image_add_first(struct image *const image, struct tile *const tile) {
  int c = image->side / 2;
  int idx = c * image->side + c;
  image->tiles[idx] = tile;
  image->transforms[idx].flip = FLIP_NONE;
  image->transforms[idx].rotation = ROTATION_NONE;
}
static void image_displace(struct image *const image, const enum direction dir) {
  int diffx = 0;
  int diffy = 0;

  DBG("DISPLACE IMAGE %s", (const char *[]){"UP", "RIGHT", "DOWN", "LEFT"}[dir]);
  switch (dir) {
  case UP:
    for (int i = 0; i < image->side; i++) {
      ASSERT(*idxtile_const(image, i, 0) == NULL, "nonempty removed row");
    }
    diffy = +1;
    break;
  case RIGHT:
    for (int i = 0; i < image->side; i++) {
      ASSERT(*idxtile_const(image, image->side - 1, i) == NULL, "nonempty removed column");
    }
    diffx = -1;
    break;
  case DOWN:
    for (int i = 0; i < image->side; i++) {
      ASSERT(*idxtile_const(image, i, image->side - 1) == NULL, "nonempty removed row");
    }
    diffy = -1;
    break;
  case LEFT:
    for (int i = 0; i < image->side; i++) {
      ASSERT(*idxtile_const(image, 0, i) == NULL, "nonempty removed column");
    }
    diffx = +1;
    break;
  default:
    FAIL("unexpected direction");
  }

  const struct tile **const newtiles = malloc(image->side * image->side * sizeof *image->tiles);
  struct transform *const newtiles_transforms = malloc(image->side * image->side * sizeof *image->transforms);
  for (int j = 0; j < image->side; j++) {
    for (int i = 0; i < image->side; i++) {
      int idx = j * image->side + i;
      newtiles[idx] = NULL;
      if (i + diffx >= image->side || j + diffy >= image->side || i + diffx < 0 || j + diffy < 0) {
        continue;
      }

      newtiles[idx] = *idxtile(image, i + diffx, j + diffy);
      newtiles_transforms[idx] = *idxtrans(image, i + diffx, j + diffy);
    }
  }
  free(image->tiles);
  free(image->transforms);
  image->tiles = newtiles;
  image->transforms = newtiles_transforms;
}

static void get_border(const struct tile *const tile, const struct transform transform, const enum direction direction,
                       char *const border) {
  // a big fat hardcoded table sounds like the easiest and fastest way to do it

  ASSERT(transform.flip != FLIP_LAST && transform.rotation != ROTATION_LAST, "unexpected flip or rotation %d",
         tile->id);

  //           FLIP N     FLIP H
  //
  //            A B C      C B A
  //   ROT 0    D E F      F E D
  //            G H I      I H G
  //
  //            G D A      A D G
  //  ROT 90    H E B      B E H
  //            I F C      C F I
  //
  //            C F I      I F C
  // ROT 270    B E H      H E B
  //            A D G      G D A
  //
  //            I H G      G H I
  // ROT 180    F E D      D E F
  //            C B A      A B C

#define ADG .start_x = 0, .start_y = 0, .inc_x = +TILE_SIDE, .inc_y = +1
#define ABC .start_x = 0, .start_y = 0, .inc_x = +1, .inc_y = +TILE_SIDE
#define GDA .start_x = 0, .start_y = TILE_SIDE - 1, .inc_x = +TILE_SIDE, .inc_y = -1
#define GHI .start_x = 0, .start_y = TILE_SIDE - 1, .inc_x = +1, .inc_y = -TILE_SIDE
#define CFI .start_x = TILE_SIDE - 1, .start_y = 0, .inc_x = -TILE_SIDE, .inc_y = +1
#define CBA .start_x = TILE_SIDE - 1, .start_y = 0, .inc_x = -1, .inc_y = +TILE_SIDE
#define IFC .start_x = TILE_SIDE - 1, .start_y = TILE_SIDE - 1, .inc_x = -TILE_SIDE, .inc_y = -1
#define IHG .start_x = TILE_SIDE - 1, .start_y = TILE_SIDE - 1, .inc_x = -1, .inc_y = -TILE_SIDE

  static const struct {
    int start_x, start_y;
    int inc_x, inc_y;
  } table[4][4][3] = {
      {
          // DIRECTION = UP
          {
              // ROTATION = NONE
              // FLIP = NONE
              {ABC},
              // FLIP = HORIZONTAL
              {CBA},
          },
          {
              // ROTATION = CLOCKWISE
              // FLIP = NONE
              {GDA},
              // FLIP = HORIZONTAL
              {ADG},
          },
          {
              // ROTATION = HALFWAY
              // FLIP = NONE
              {IHG},
              // FLIP = HORIZONTAL
              {GHI},
          },
          {
              // ROTATION = COUNTERCLOCKWISE
              // FLIP = NONE
              {CFI},
              // FLIP = HORIZONTAL
              {IFC},
          },
      },
      {
          // DIRECTION = RIGHT
          {
              // ROTATION = NONE
              // FLIP = NONE
              {CFI},
              // FLIP = HORIZONTAL
              {ADG},
          },
          {
              // ROTATION = CLOCKWISE
              // FLIP = NONE
              {ABC},
              // FLIP = HORIZONTAL
              {GHI},
          },
          {
              // ROTATION = HALFWAY
              // FLIP = NONE
              {GDA},
              // FLIP = HORIZONTAL
              {IFC},
          },
          {
              // ROTATION = COUNTERCLOCKWISE
              // FLIP = NONE
              {IHG},
              // FLIP = HORIZONTAL
              {CBA},
          },
      },
      {
          // DIRECTION = DOWN
          {
              // ROTATION = NONE
              // FLIP = NONE
              {GHI},
              // FLIP = HORIZONTAL
              {IHG},
          },
          {
              // ROTATION = CLOCKWISE
              // FLIP = NONE
              {IFC},
              // FLIP = HORIZONTAL
              {CFI},
          },
          {
              // ROTATION = HALFWAY
              // FLIP = NONE
              {CBA},
              // FLIP = HORIZONTAL
              {ABC},
          },
          {
              // ROTATION = COUNTERCLOCKWISE
              // FLIP = NONE
              {ADG},
              // FLIP = HORIZONTAL
              {GDA},
          },
      },
      {
          // DIRECTION = LEFT
          {
              // ROTATION = NONE
              // FLIP = NONE
              {ADG},
              // FLIP = HORIZONTAL
              {CFI},
          },
          {
              // ROTATION = CLOCKWISE
              // FLIP = NONE
              {GHI},
              // FLIP = HORIZONTAL
              {ABC},
          },
          {
              // ROTATION = HALFWAY
              // FLIP = NONE
              {IFC},
              // FLIP = HORIZONTAL
              {GDA},
          },
          {
              // ROTATION = COUNTERCLOCKWISE
              // FLIP = NONE
              {CBA},
              // FLIP = HORIZONTAL
              {IHG},
          },
      },
  };

  int start_x = table[direction][transform.rotation][transform.flip].start_x;
  int start_y = table[direction][transform.rotation][transform.flip].start_y;
  int inc_x = table[direction][transform.rotation][transform.flip].inc_x;
  int inc_y = table[direction][transform.rotation][transform.flip].inc_y;

  int i = 0;
  for (int y = start_y; y < TILE_SIDE && y >= 0; y += inc_y) {
    for (int x = start_x; x < TILE_SIDE && x >= 0; x += inc_x) {
      char pixel = *idxpixel_const(tile, x, y);
      border[i++] = pixel;
    }
  }
}

static bool fit_frame(const struct tile *const tiles[4], const struct transform transforms[4],
                      const struct tile *const newtile, struct transform *const newtile_transform) {
  char goals[4][TILE_SIDE];
  for (int i = 0; i < 4; i++) {
    if (tiles[i] != NULL) {
      get_border(tiles[i], transforms[i], (i + 2) % 4, goals[i]);
      // DBG("Trying to match %d, possible neighbor %d has %c%c%c%c%c%c%c%c%c%c border in direction %s", newtile->id,
      // tiles[i]->id, goals[i][0], goals[i][1], goals[i][2], goals[i][3], goals[i][4], goals[i][5], goals[i][6],
      // goals[i][7], goals[i][8], goals[i][9], (const char *[]){"UP","RIGHT","DOWN","LEFT"}[(i+2)%4]);
    }
  }

  for (enum rotation rot = ROTATION_NONE; rot < ROTATION_LAST; rot++) {
    for (enum flip flip = FLIP_NONE; flip < FLIP_LAST; flip++) {
      struct transform possible_transform = {
          .rotation = rot,
          .flip = flip,
      };
      // DBG("Try transform: %s rot %s flip", (const char
      // *[]){"no","clock","half","ctrclock"}[possible_transform.rotation], (const char
      // *[]){"no","horiz"}[possible_transform.flip]);

      bool all = true;
      for (int i = 0; i < 4; i++) {
        if (tiles[i] == NULL) {
          continue;
        }

        char this_border[TILE_SIDE];
        get_border(newtile, possible_transform, i, this_border);
        if (strncmp(goals[i], this_border, TILE_SIDE) != 0) {
          all = false;
          break;
        } else {
          // DBG("%c%c%c%c%c%c%c%c%c%c (%d) (%s) == %c%c%c%c%c%c%c%c%c%c (%d) (%s)", goals[i][0], goals[i][1],
          // goals[i][2], goals[i][3], goals[i][4], goals[i][5], goals[i][6], goals[i][7], goals[i][8], goals[i][9],
          // tiles[i]->id, (const char *[]){"UP","RIGHT","DOWN","LEFT"}[(i+2)%4], this_border[0], this_border[1],
          // this_border[2], this_border[3], this_border[4], this_border[5], this_border[6], this_border[7],
          // this_border[8], this_border[9], newtile->id, (const char *[]){"UP","RIGHT","DOWN","LEFT"}[i]);
        }
      }

      if (all) {
        *newtile_transform = possible_transform;
        return true;
      }
    }
  }

  return false;
}

static bool find_position(const struct image *const image, const struct tile *const tile, int *const x, int *const y,
                          struct transform *const transform) {

  for (int j = -1; j <= image->side; j++) {
    for (int i = -1; i <= image->side; i++) {
      const struct tile *candidate;
      if (j < 0 || i < 0 || j >= image->side || i >= image->side) {
        candidate = NULL;
      } else {
        candidate = *idxtile_const(image, i, j);
      }

      if (candidate == NULL) {
        const struct tile *neighbors[4];
        struct transform neighbor_trans[4];
        if (i > 0 && j >= 0 && j < image->side) {
          neighbors[LEFT] = *idxtile_const(image, i - 1, j);
          neighbor_trans[LEFT] = *idxtrans_const(image, i - 1, j);
        } else {
          neighbors[LEFT] = NULL;
        }
        if (j > 0 && i >= 0 && i < image->side) {
          neighbors[UP] = *idxtile_const(image, i, j - 1);
          neighbor_trans[UP] = *idxtrans_const(image, i, j - 1);
        } else {
          neighbors[UP] = NULL;
        }
        if (j < image->side - 1 && i >= 0 && i < image->side) {
          neighbors[DOWN] = *idxtile_const(image, i, j + 1);
          neighbor_trans[DOWN] = *idxtrans_const(image, i, j + 1);
        } else {
          neighbors[DOWN] = NULL;
        }
        if (i < image->side - 1 && j >= 0 && j < image->side) {
          neighbors[RIGHT] = *idxtile_const(image, i + 1, j);
          neighbor_trans[RIGHT] = *idxtrans_const(image, i + 1, j);
        } else {
          neighbors[RIGHT] = NULL;
        }

        bool allnull = true;
        for (int k = 0; k < 4; k++) {
          if (neighbors[k] != NULL) {
            allnull = false;
            break;
          }
        }
        if (allnull) {
          continue;
        }

        if (fit_frame(neighbors, neighbor_trans, tile, transform)) {
          *x = i;
          *y = j;
          return true;
        }
      }
    }
  }
  return false;
}

static bool image_add_tile(struct image *const image, struct tile *const tile) {
  int x, y;
  struct transform transform;
  bool found = find_position(image, tile, &x, &y, &transform);
  if (!found) {
    return false;
  }

  bool displaced = false;
  if (x < 0) {
    ASSERT(x == -1, "should not have to displace more than once");
    image_displace(image, RIGHT);
    displaced = true;
    x++;
  }
  if (x >= image->side) {
    ASSERT(x == image->side, "should not have to displace more than once");
    ASSERT(!displaced, "should not have to displace more than once");
    image_displace(image, LEFT);
    displaced = true;
    x--;
  }
  if (y < 0) {
    ASSERT(y == -1, "should not have to displace more than once");
    image_displace(image, DOWN);
    displaced = true;
    y++;
  }
  if (y >= image->side) {
    ASSERT(y == image->side, "should not have to displace more than once");
    ASSERT(!displaced, "should not have to displace more than once");
    image_displace(image, UP);
    displaced = true;
    y--;
  }

  *idxtile(image, x, y) = tile;
  *idxtrans(image, x, y) = transform;
  return true;
}

static int parse_int(const char **const input) {
  int n = 0;
  char c;
  while (isdigit(c = **input)) {
    n = n * 10 + c - '0';
    *input += 1;
  }
  return n;
}

static void parse_tile(const char **const input, struct tile *const tile) {
  const char *const tile_text = "Tile ";
  const size_t len_title_text = strlen(tile_text);
  ASSERT(strncmp(*input, tile_text, len_title_text) == 0, "parse error");
  *input += len_title_text;

  tile->id = parse_int(input);
  ASSERT(**input == ':', "parse error");
  *input += 1;
  ASSERT(**input == '\n', "parse error");
  *input += 1;

  for (int j = 0; j < TILE_SIDE; j++) {
    for (int i = 0; i < TILE_SIDE; i++) {
      char *pixel = idxpixel(tile, i, j);
      *pixel = **input;
      ASSERT(*pixel == '.' || *pixel == '#', "parse error");
      *input += 1;
    }
    ASSERT(**input == '\n', "parse error");
    *input += 1;
  }
}

static struct tile *parse(const char *input, size_t *const len) {
  size_t capacity = 16;
  struct tile *list = malloc(capacity * sizeof *list);
  *len = 0;

  for (; *input != '\0'; input++) {
    if (*len >= capacity) {
      capacity *= 2;
      list = realloc(list, capacity * sizeof *list);
    }

    parse_tile(&input, list + *len);
    *len += 1;

    if (*input == '\0') {
      break;
    }
    ASSERT(*input == '\n', "parse error");
  }

  ASSERT(sqrt(*len) - (int)sqrt(*len) == 0, "parse error");
  return list;
}

static void create_image(struct tile *const tiles, const size_t tiles_len, struct image *const image) {
  image_init(image, (int)sqrt(tiles_len));

  image_add_first(image, tiles + 0);

  bool added_tiles[tiles_len];
  memset(added_tiles, 0, tiles_len * sizeof(bool));
  added_tiles[0] = true;

  bool done = false;

  while (!done) {
#ifdef DEBUG
    for (int j = 0; j < image->side; j++) {
      for (int k = 0; k < image->side; k++) {
        fprintf(stderr, "%d ", *idxtile_const(image, k, j) != NULL);
      }
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
#endif

    done = true;
    bool added = false;
    for (size_t i = 0; i < tiles_len; i++) {
      if (!added_tiles[i]) {
        done = false;
        added_tiles[i] = image_add_tile(image, tiles + i);
        if (added_tiles[i]) {
          added = true;

#ifdef DEBUG
          for (int j = 0; j < image->side; j++) {
            for (int k = 0; k < image->side; k++) {
              fprintf(stderr, "%d ", *idxtile_const(image, k, j) != NULL);
            }
            fprintf(stderr, "\n");
          }
          fprintf(stderr, "\n");
#endif
        }
      }
    }
    ASSERT(done || added, "nothing added in iteration");
  }
}

static void *memdup(const void *const src, const size_t size) {
  void *dst = malloc(size);
  return memcpy(dst, src, size);
}

static void transform(char *img, enum rotation rotation, enum flip flip, int side) {
  size_t size = side * side * sizeof(char);
  char *img1 = memdup(img, size);
  char *img2 = malloc(size);

  if (rotation != ROTATION_NONE) {
    for (int l = 0; l < side; l++) {
      for (int k = 0; k < side; k++) {
        int kk, ll;
        switch (rotation) {
        case ROTATION_CLOCKWISE:
          kk = side - 1 - l;
          ll = k;
          break;
        case ROTATION_COUNTERCLOCKWISE:
          kk = l;
          ll = side - 1 - k;
          break;
        case ROTATION_HALF:
          kk = side - 1 - k;
          ll = side - 1 - l;
          break;
        case ROTATION_NONE:
        case ROTATION_LAST:
        default:
          FAIL("invalid rotation");
          break;
        }

        img2[ll * side + kk] = img1[l * side + k];
      }
    }
    char *tmp = img1;
    img1 = img2;
    img2 = tmp;
  }

  if (flip != FLIP_NONE) {
    for (int l = 0; l < side; l++) {
      for (int k = 0; k < side; k++) {
        int kk, ll;
        switch (flip) {
        case FLIP_HORIZONTAL:
          kk = side - 1 - k;
          ll = l;
          break;
        case FLIP_NONE:
        case FLIP_LAST:
        default:
          FAIL("invalid flip");
        }

        img2[ll * side + kk] = img1[l * side + k];
      }
    }

    char *tmp = img1;
    img1 = img2;
    img2 = tmp;
  }

  memcpy(img, img1, size);
  free(img1);
  free(img2);
}

static void copy_image_over(const struct image *const image, char *const final, const int final_side) {
  for (int j = 0; j < image->side; j++) {
    for (int i = 0; i < image->side; i++) {
      const struct tile *const tile = *idxtile_const(image, i, j);
      const struct transform trans = *idxtrans_const(image, i, j);

      DBG("tile: %d", tile->id);
      DBG("rotation: %s", (const char *[]){"NONE", "CLOCKWISE", "HALF", "COUNTERCLOCKWISE"}[trans.rotation]);
      DBG("flip: %s\n", (const char *[]){"NONE", "HORIZONTAL", "VERTICAL"}[trans.flip]);

      static char transformed_tile[TILE_SIDE * TILE_SIDE];

      for (int l = 0; l < TILE_SIDE; l++) {
        for (int k = 0; k < TILE_SIDE; k++) {
          transformed_tile[l * TILE_SIDE + k] = *idxpixel_const(tile, k, l);
        }
      }

      transform(transformed_tile, trans.rotation, trans.flip, TILE_SIDE);

      int x = i * (TILE_SIDE - 2);
      int y = j * (TILE_SIDE - 2);
      for (int l = 1; l < TILE_SIDE - 1; l++) {
        for (int k = 1; k < TILE_SIDE - 1; k++) {
          final[(y + l - 1) * final_side + (x + k - 1)] = transformed_tile[l * TILE_SIDE + k];
        }
      }
    }
  }
}

static void solution1(const char *const input, char *const output) {
  size_t tiles_len;
  struct tile *tiles = parse(input, &tiles_len);

  struct image image;
  create_image(tiles, tiles_len, &image);

  long result = 1;
  result *= (*idxtile_const(&image, 0, 0))->id;
  result *= (*idxtile_const(&image, 0, image.side - 1))->id;
  result *= (*idxtile_const(&image, image.side - 1, 0))->id;
  result *= (*idxtile_const(&image, image.side - 1, image.side - 1))->id;

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
  image_free(&image);
  free(tiles);
}

static bool match_pattern(char *const bigimg, int x, int y, int side, const char *const pattern, const int px,
                          const int py) {
  bool match = true;

  for (int j = 0; j < py; j++) {
    for (int i = 0; i < px; i++) {
      int ii = x + i;
      int jj = y + j;
      if (pattern[j * px + i] != ' ') {
        if (bigimg[jj * side + ii] != pattern[j * px + i]) {
          match = false;
          goto end;
        }
      }
    }
  }

  if (match) {
    for (int j = 0; j < py; j++) {
      for (int i = 0; i < px; i++) {
        int ii = x + i;
        int jj = y + j;
        if (pattern[j * px + i] != ' ') {
          bigimg[jj * side + ii] = 'O';
        }
      }
    }
  }

end:
  return match;
}

static int mark_monsters(char *const img, int side) {
  static const char *const pattern = "                  # "
                                     "#    ##    ##    ###"
                                     " #  #  #  #  #  #   ";
  static const int patternX = 20;
  static const int patternY = 3;

  int count = 0;
  for (int j = 0; j <= side - patternY; j++) {
    for (int i = 0; i <= side - patternX; i++) {
      bool found = match_pattern(img, i, j, side, pattern, patternX, patternY);
      if (found) {
        count++;
      }
    }
  }

  return count;
}

static void solution2(const char *const input, char *const output) {
  size_t tiles_len;
  struct tile *tiles = parse(input, &tiles_len);

  struct image image;
  create_image(tiles, tiles_len, &image);

  int final_image_side = image.side * (TILE_SIDE - 2);
  size_t final_image_size = final_image_side * final_image_side * sizeof(char);
  char *final_image = malloc(final_image_size);
  copy_image_over(&image, final_image, final_image_side);

  image_free(&image);
  free(tiles);

#ifdef DEBUG
  for (int j = 0; j < final_image_side; j++) {
    for (int i = 0; i < final_image_side; i++) {
      fputc(final_image[j * final_image_side + i], stderr);
    }
    fputc('\n', stderr);
  }
  fputc('\n', stderr);
#endif

  char *imgs[8];
  for (int i = 0; i < 8; i++) {
    imgs[i] = memdup(final_image, final_image_size);
  }
  free(final_image);

  for (int i = 0; i < 4; i++) {
    transform(imgs[i], i, FLIP_NONE, final_image_side);
    transform(imgs[4 + i], i, FLIP_HORIZONTAL, final_image_side);
  }

  char *bestimg = NULL;
  int bestcount = 0;
  for (int i = 0; i < 8; i++) {
    int count = mark_monsters(imgs[i], final_image_side);
    if (count > bestcount) {
      bestcount = count;
      bestimg = imgs[i];
    }

#ifdef DEBUG
    fprintf(stderr, "img %d\n", i);
    for (int y = 0; y < final_image_side; y++) {
      for (int x = 0; x < final_image_side; x++) {
        fputc(imgs[i][y * final_image_side + x], stderr);
      }
      fputc('\n', stderr);
    }
    fputc('\n', stderr);
#endif
  }

  int result = 0;
  for (int j = 0; j < final_image_side; j++) {
    for (int i = 0; i < final_image_side; i++) {
      char pixel = bestimg[j * final_image_side + i];
      if (pixel == '#') {
        result++;
      }
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
  for (int i = 0; i < 8; i++) {
    free(imgs[i]);
  }
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
