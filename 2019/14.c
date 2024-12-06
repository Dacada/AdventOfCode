#include <aoclib.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MATERIALS_BASE ('Z' - 'A' + 1UL)
#define MATERIALS_BASE_5 (MATERIALS_BASE * MATERIALS_BASE * MATERIALS_BASE * MATERIALS_BASE * MATERIALS_BASE)
#define MATERIALS_BASE_4 (MATERIALS_BASE * MATERIALS_BASE * MATERIALS_BASE * MATERIALS_BASE)
#define MATERIALS_BASE_3 (MATERIALS_BASE * MATERIALS_BASE * MATERIALS_BASE)
#define MATERIALS_BASE_2 (MATERIALS_BASE * MATERIALS_BASE)
#define MATERIALS_BASE_1 (MATERIALS_BASE)
#define MATERIALS_BASE_0 (1)
#define TOTAL_POSSIBLE_MATERIALS MATERIALS_BASE_5

#define INDEX(str)                                                                                                     \
  ((str[0] - 'A') * MATERIALS_BASE_4 + (str[1] - 'A') * MATERIALS_BASE_3 + (str[2] - 'A') * MATERIALS_BASE_2 +         \
   (str[3] - 'A') * MATERIALS_BASE_1 + (str[4] - 'A') * MATERIALS_BASE_0)

#ifdef DEBUG
#define UNINDEX(str, num)                                                                                              \
  do {                                                                                                                 \
    str[0] = num / MATERIALS_BASE_4 % MATERIALS_BASE + 'A';                                                            \
    str[1] = num / MATERIALS_BASE_3 % MATERIALS_BASE + 'A';                                                            \
    str[2] = num / MATERIALS_BASE_2 % MATERIALS_BASE + 'A';                                                            \
    str[3] = num / MATERIALS_BASE_1 % MATERIALS_BASE + 'A';                                                            \
    str[4] = num / MATERIALS_BASE_0 % MATERIALS_BASE + 'A';                                                            \
    str[5] = '\0';                                                                                                     \
  } while (0)
#else
#define UNINDEX(str, num)                                                                                              \
  do {                                                                                                                 \
  } while (0)
#endif

#define CEILDIV(a, b) ((a + b - 1) / b)

#define EXPECT_INPUT(i, exp, got)                                                                                      \
  ASSERT(exp == got, "Unexpected input: Character %lu. Expected '%c' (%d) but got '%c' (%d)", i, exp, exp, got, got)

// A pointer to first element of array of ingridients, a pointer to
// first element of array of quantities and the number of elemements
// in the recipe such that the a recipe requires quantities[i]
// ingridients[i] for 1 >= i < total_elements and creates a total of
// result whatever it makes.
struct recipe {
  uint32_t *ingridients; // Our codification for ingridients
                         // fits within a 32 bit unsigned
                         // integer, 5 numbers from A to Z
                         // (padded with As if less than 5
                         // characters, e.g. AFUEL or AAORE)
  unsigned *quantities;
  unsigned total_elements;
  unsigned result;
};

// A collection of many recipes.
struct recipe_collection {
  struct recipe *recipes;
  unsigned total;
};

static struct recipe_collection materials[TOTAL_POSSIBLE_MATERIALS];
static long material_amounts[TOTAL_POSSIBLE_MATERIALS];

static void parse_ingridient(const char *const input, size_t *const i, uint32_t *const ingridient,
                             unsigned *const quantity) {
  *quantity = 0;
  while (input[*i] >= '0' && input[*i] <= '9') {
    *quantity *= 10;
    *quantity += input[*i] - '0';
    ++*i;
  }
  EXPECT_INPUT(*i, ' ', input[*i]);
  ++*i;

  *ingridient = 0;
  while (input[*i] >= 'A' && input[*i] <= 'Z') {
    *ingridient *= MATERIALS_BASE;
    *ingridient += input[*i] - 'A';
    ++*i;
  }
}

static void parse_input_line(const char *const input, size_t *const i) {
  static uint32_t ingridients_buffer[128];
  static int quantities_buffer[128];
  int total_found = 0;

  uint32_t ingridient;
  unsigned quantity;

  for (;;) {
    parse_ingridient(input, i, &ingridient, &quantity);

    ingridients_buffer[total_found] = ingridient;
    quantities_buffer[total_found] = quantity;
    total_found++;

    if (input[*i] == ',') {
      ++*i;
      EXPECT_INPUT(*i, ' ', input[*i]);
      ++*i;
    } else if (input[*i] == ' ') {
      ++*i;
      EXPECT_INPUT(*i, '=', input[*i]);
      ++*i;
      EXPECT_INPUT(*i, '>', input[*i]);
      ++*i;
      EXPECT_INPUT(*i, ' ', input[*i]);
      ++*i;
      break;
    } else {
      FAIL("Unexpected input: Expected ',' or ' ' but got '%c' (%d)", input[*i], input[*i]);
    }
  }

  parse_ingridient(input, i, &ingridient, &quantity);

  struct recipe *r = materials[ingridient].recipes;
  unsigned j = materials[ingridient].total;
  r = realloc(r, sizeof(*r) * (++materials[ingridient].total));
  if (r == NULL) {
    perror("realloc");
    abort();
  }
  materials[ingridient].recipes = r;

  r[j].ingridients = malloc(sizeof(uint32_t) * total_found);
  r[j].quantities = malloc(sizeof(unsigned) * total_found);
  for (int k = 0; k < total_found; k++) {
    r[j].ingridients[k] = ingridients_buffer[k];
    r[j].quantities[k] = quantities_buffer[k];
  }
  r[j].total_elements = total_found;
  r[j].result = quantity;
}

static void parse_input(const char *const input) {
  for (size_t i = 0; i < TOTAL_POSSIBLE_MATERIALS; i++) {
    materials[i].recipes = NULL;
    materials[i].total = 0;
  }

  size_t i = 0;
  do {
    parse_input_line(input, &i);
  } while (input[i++] == '\n' && input[i] != '\0');
}

static void create(long amount, uint32_t material) {
#ifdef DEBUG
  char tmp[6];
#endif

  if (material == INDEX("AAORE")) {
    return;
  }

  UNINDEX(tmp, material);
  // DBG("Trying to create %ld %s", amount, tmp);

  if (material_amounts[material] >= amount) {
    // DBG("We already have enough in reserve (%ld) so we do nothing.",
    //     material_amounts[material]);
    return;
  }

  struct recipe_collection *recipes = &materials[material];
  ASSERT(recipes->total == 1, "Expected all materials to have only one recipe (offending material: %s)", tmp);

  struct recipe *recipe = recipes->recipes;

  long amount_needed = amount - material_amounts[material];
  long applications_needed = CEILDIV(amount_needed, recipe->result);

  for (unsigned i = 0; i < recipe->total_elements; i++) {
    create(recipe->quantities[i] * applications_needed, recipe->ingridients[i]);

    UNINDEX(tmp, recipe->ingridients[i]);
    // DBG("We've created at least %u * %ld %s in order to create our objective material.",
    //     recipe->quantities[i], applications_needed, tmp);

    material_amounts[recipe->ingridients[i]] -= recipe->quantities[i] * applications_needed;
    // DBG("And now we've substracted %u * %ld from the pool to use it for our many material creation.",
    //     recipe->quantities[i], applications_needed);
  }

  UNINDEX(tmp, material);
  // DBG("We add to the material pool the result of this recipe: %u %s", recipe->result, tmp);
  material_amounts[material] += recipe->result * applications_needed;
}

static void solution1(const char *const input, char *const output) {
  parse_input(input);
  material_amounts[INDEX("AAORE")] = 0;
  create(1, INDEX("AFUEL"));
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", -material_amounts[INDEX("AAORE")]);
}

static void solution2(const char *const input, char *const output) {
  parse_input(input);

  const long one_trillion = 1000000000000L;

  long low = 1;
  long high = one_trillion;
  long bound;
  enum { DESCENDING, ASCENDING } direction = DESCENDING;

  for (;;) {
    if (high - low == 1) {
      if (direction == ASCENDING) {
        memset(material_amounts, 0, sizeof(long) * TOTAL_POSSIBLE_MATERIALS);
        create(high, INDEX("AFUEL"));
        long got = -material_amounts[INDEX("AAORE")];
        if (got < one_trillion) {
          bound = high;
        } else {
          bound = low;
        }
      } else {
        bound = low;
      }
      break;
    }

    bound = (high + low) / 2;
    DBG("Testing between %ld and %ld (%ld)", low, high, bound);

    memset(material_amounts, 0, sizeof(long) * TOTAL_POSSIBLE_MATERIALS);
    create(bound, INDEX("AFUEL"));

    long got = -material_amounts[INDEX("AAORE")];
    DBG("Got %ld", got);

    if (got > one_trillion) {
      high = bound;
      direction = DESCENDING;
    } else if (got < one_trillion) {
      low = bound;
      direction = ASCENDING;
    } else {
      break;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", bound);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
