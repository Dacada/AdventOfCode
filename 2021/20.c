#include <aoclib.h>
#include <stdio.h>

#define ALGOSIZE 512

// The image is represented as an array of 9 bit numbers such that image[y][x]
// is the 3x3 chunk that starts at the y,x of the real image. To deal with
// infinity we set an additional bit indicating what's in the "sea of infinity"
// (either all lit or all unlit). Of course the chunks in this array
// overlap. To reproduce the image one would only take the 4th bit of each of
// the chunks, see the diagram below.

// 0 1 2
// 3 4 5
// 6 7 8
// 0b012345678

struct image {
  unsigned *chunks;
  size_t width;
  size_t height;
  bool lit;
};

static void parse_algo(const char **const input, bool algo[ALGOSIZE]) {
  for (unsigned i = 0; i < ALGOSIZE; i++) {
    char c = **input;
    *input += 1;
    ASSERT(c == '#' || c == '.', "parse error");
    algo[i] = c == '#';
  }
  ASSERT(**input == '\n', "parse error");
  *input += 1;
  ASSERT(**input == '\n', "parse error");
  *input += 1;
}

static void assign_pixel(unsigned *const chunks, size_t width, size_t j, size_t i, bool pixel) {
  if (pixel) {
    chunks[(j + 0) * width + (i + 0)] |= 1 << 0;
    chunks[(j + 0) * width + (i + 1)] |= 1 << 1;
    chunks[(j + 0) * width + (i + 2)] |= 1 << 2;
    chunks[(j + 1) * width + (i + 0)] |= 1 << 3;
    chunks[(j + 1) * width + (i + 1)] |= 1 << 4;
    chunks[(j + 1) * width + (i + 2)] |= 1 << 5;
    chunks[(j + 2) * width + (i + 0)] |= 1 << 6;
    chunks[(j + 2) * width + (i + 1)] |= 1 << 7;
    chunks[(j + 2) * width + (i + 2)] |= 1 << 8;
  } else {
    chunks[(j + 0) * width + (i + 0)] &= ~(1 << 0);
    chunks[(j + 0) * width + (i + 1)] &= ~(1 << 1);
    chunks[(j + 0) * width + (i + 2)] &= ~(1 << 2);
    chunks[(j + 1) * width + (i + 0)] &= ~(1 << 3);
    chunks[(j + 1) * width + (i + 1)] &= ~(1 << 4);
    chunks[(j + 1) * width + (i + 2)] &= ~(1 << 5);
    chunks[(j + 2) * width + (i + 0)] &= ~(1 << 6);
    chunks[(j + 2) * width + (i + 1)] &= ~(1 << 7);
    chunks[(j + 2) * width + (i + 2)] &= ~(1 << 8);
  }
}

static void parse_image(const char *input, struct image *const image) {
  const char *const originput = input;

  image->width = 0;
  while (*input != '\n') {
    input += 1;
    image->width += 1;
  }

  image->height = 0;
  while (*input != '\0') {
    input += image->width;
    image->height += 1;
    if (*input == '\n' && *(input + 1) == '\0') {
      break;
    }
  }

  size_t inwidth = image->width + 1; // +1 for linebreaks
  size_t inheight = image->height;

  // helper function expects larger image and i don't want to add an
  // exception
  image->height += 2;
  image->width += 2;

  input = originput;
  image->chunks = malloc(sizeof(*image->chunks) * image->width * image->height);
  for (size_t i = 0; i < image->width * image->height; i++) {
    image->chunks[i] = 0;
  }
  image->lit = false;

  for (size_t j = 0; j < inheight; j++) {
    for (size_t i = 0; i < inwidth - 1; i++) { // -1 to ignore linebreaks
      char c = input[j * inwidth + i];
      ASSERT(c == '#' || c == '.', "parse error '%s'", input + j * inwidth + i - 1);

      bool pixel = c == '#';
      assign_pixel(image->chunks, image->width, j, i, pixel);
    }
  }
}

static void run_algo(struct image *const image, bool algo[ALGOSIZE]) {
  size_t new_width = image->width + 2;
  size_t new_height = image->height + 2;

  bool new_lit;
  if (algo[0] && !image->lit) {
    new_lit = true;
  } else if (!algo[ALGOSIZE - 1] && image->lit) {
    new_lit = false;
  } else {
    new_lit = image->lit;
  }

  unsigned *new_chunks = malloc(sizeof(*new_chunks) * new_height * new_width);
  for (size_t i = 0; i < new_height * new_width; i++) {
    if (new_lit) {
      new_chunks[i] = ALGOSIZE - 1;
    } else {
      new_chunks[i] = 0;
    }
  }

  for (size_t j = 0; j < image->height; j++) {
    for (size_t i = 0; i < image->width; i++) {
      unsigned idx = image->chunks[j * image->width + i];

      bool pixel = algo[idx];
      assign_pixel(new_chunks, new_width, j, i, pixel);
    }
  }

  free(image->chunks);
  image->chunks = new_chunks;
  image->height = new_height;
  image->width = new_width;
  image->lit = new_lit;
}

__attribute__((pure)) static unsigned count_pixels(const struct image *const image) {
  ASSERT(!image->lit, "count infinite pixels!");
  unsigned count = 0;
  for (size_t j = 0; j < image->height; j++) {
    for (size_t i = 0; i < image->width; i++) {
      count += (bool)(image->chunks[j * image->width + i] & (1 << 4));
    }
  }
  return count;
}

static void prettyprint_image(const struct image *const image) {
#ifdef DEBUG
  for (size_t j = 0; j < image->height; j++) {
    for (size_t i = 0; i < image->width; i++) {
      bool pixel = image->chunks[j * image->width + i] & (1 << 4);
      if (pixel) {
        fprintf(stderr, "#");
      } else {
        fprintf(stderr, ".");
      }
    }
    fprintf(stderr, "\n");
  }
#else
  (void)image;
#endif
}

static void solution1(const char *input, char *const output) {
  static bool algo[ALGOSIZE];
  parse_algo(&input, algo);

  struct image image;
  parse_image(input, &image);

  prettyprint_image(&image);
  run_algo(&image, algo);
  prettyprint_image(&image);
  run_algo(&image, algo);
  prettyprint_image(&image);

  unsigned res = count_pixels(&image);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", res);
  free(image.chunks);
}

static void solution2(const char *input, char *const output) {
  static bool algo[ALGOSIZE];
  parse_algo(&input, algo);

  struct image image;
  parse_image(input, &image);

  for (int i = 0; i < 50; i++) {
    run_algo(&image, algo);
  }
  unsigned res = count_pixels(&image);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", res);
  free(image.chunks);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
