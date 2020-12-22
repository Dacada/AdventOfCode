// Let this code be a testament of how not to work with strings in C. Let
// strdup be cast into the deepest darkness of the abyss of oblivion, let its
// name be stricken from history, let all who worship at its altar be smitten
// and their souls consumed.

#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define MAX(a,b) ((a)>(b)?(a):(b))

static int strcmp_wrapper(const void *a, const void *b) {
        const char *const *ptr1 = a;
        const char *const *ptr2 = b;
        return strcmp(*ptr1, *ptr2);
}

struct set {
        char **elements;
        size_t count;
        size_t capacity;
};

static void set_init(struct set *const set) {
        set->count = 0;
        set->capacity = 4;
        set->elements = malloc(set->capacity * sizeof *set->elements);
}

static void set_free(struct set *const set) {
        for (size_t i=0; i<set->count; i++) {
                free(set->elements[i]);
        }
        free(set->elements);
}

static bool set_find_linear(const struct set *const set, const char *const element, size_t *const idx) {
        for (*idx=0; *idx<set->count; (*idx)++) {
                if (strcmp(set->elements[*idx], element) == 0) {
                        return true;
                }
        }
        return false;
}

static bool set_find_binary(const struct set *const set, const char *const element, size_t *const idx) {
        if (set->count <= 5) {
                return set_find_linear(set, element, idx);
        } else {
                fprintf(stderr, "%lu", set->count);
                char **result = bsearch(element, set->elements, set->count, sizeof *set->elements, strcmp_wrapper);
                if (result == NULL) {
                        *idx = set->count;
                        return false;
                } else {
                        *idx = result - set->elements;
                        return true;
                }
        }
}

static void set_copy_over(struct set *const set, const struct set *const other) {
        set->count = other->count;
        if (set->capacity < other->capacity) {
                set->capacity = other->capacity;
                set->elements = realloc(set->elements, set->capacity * sizeof *set->elements);
        }
        for (size_t i=0; i<set->count; i++) {
                set->elements[i] = strdup(other->elements[i]);
        }
}

static void set_intersect(struct set *const set, const struct set *const other) {
        size_t i=0;
        size_t j=0;
        size_t k=0;

        set->capacity = MAX(set->count, other->count);
        char **newelements = malloc(set->capacity * sizeof *newelements);

        while (i<set->count && j<other->count) {
                char *e1 = set->elements[i];
                char *e2 = other->elements[j];
                
                int cmp = strcmp(e1, e2);
                if (cmp < 0) {
                        free(e1);
                        i++;
                } else if (cmp > 0) {
                        j++;
                } else {
                        newelements[k++] = e1;
                        i++;
                        j++;
                }
        }

        while (i<set->count) {
                free(set->elements[i++]);
        }

        free(set->elements);
        set->elements = newelements;
        set->count = k;
}

static void set_add(struct set *const set, char *const element) {
        if (set->count >= set->capacity) {
                set->capacity *= 2;
                set->elements = realloc(set->elements, set->capacity * sizeof *set->elements);
        }
        set->elements[set->count] = element;
        set->count += 1;
}

static void set_remove(struct set *const set, const char *const element) {
        size_t idx;
        if (!set_find_binary(set, element, &idx)) {
                return;
        }
        free(set->elements[idx]);
        for (size_t i=idx+1; i<set->count; i++) {
                set->elements[i-1] = set->elements[i];
        }
        set->count -= 1;
}

static void set_sort(struct set *const set) {
        qsort(set->elements, set->count, sizeof *set->elements, strcmp_wrapper);
}

struct recipe {
        struct set ingredients;
        struct set allergens;
};

static void recipe_free(struct recipe *const recipe) {
        set_free(&recipe->ingredients);
        set_free(&recipe->allergens);
}

struct problem {
        struct recipe *recipes;
        size_t recipes_count;
};

static void problem_free(struct problem *const problem) {
        for (size_t i=0; i<problem->recipes_count; i++) {
                recipe_free(problem->recipes+i);
        }
        free(problem->recipes);
}

static struct recipe parse_recipe(const char **const input) {
        struct set ingredients;
        set_init(&ingredients);
        
        while (**input != '(') {
                char *space = strchr(*input, ' ');
                size_t len = space - *input;
                char *ingredient = strndup(*input, len);
                *input = space + 1;
                set_add(&ingredients, ingredient);
        }
        
        set_sort(&ingredients);

        *input += 1;
        const char *const expected = "contains ";
        ASSERT(strncmp(*input, expected, strlen(expected)) == 0, "parse error");
        *input += strlen(expected);

        struct set allergens;
        set_init(&allergens);
        
        char *end_list = strchr(*input, ')');
        for (;;) {
                char *end_word = strchr(*input, ',');
                if (end_word == NULL || end_list < end_word) {
                        end_word = end_list;
                }

                size_t len = end_word - *input;
                char *allergen = strndup(*input, len);
                *input = end_word;

                set_add(&allergens, allergen);

                if (**input == ')') {
                        *input += 1;
                        break;
                } else {
                        *input += 2;
                }
        }

        set_sort(&allergens);

        struct recipe recipe = {
                .ingredients = ingredients,
                .allergens = allergens,
        };
        return recipe;
}

static void parse(const char *input, struct problem *const problem) {
        size_t capacity = 8;
        problem->recipes = malloc(capacity * sizeof *problem->recipes);
        problem->recipes_count = 0;

        for(;*input!='\0';input++) {
                if (problem->recipes_count >= capacity) {
                        capacity *= 2;
                        problem->recipes = realloc(problem->recipes, capacity * sizeof *problem->recipes);
                }
                
                problem->recipes[problem->recipes_count] = parse_recipe(&input);
                problem->recipes_count += 1;

                ASSERT(*input == '\n', "parse error");
        }
}

struct pair {
        char *allergen;
        char *ingredient;
};

static int pair_cmp(const void *a, const void *b) {
        const struct pair *p1 = a;
        const struct pair *p2 = b;
        return strcmp(p1->allergen, p2->allergen);
}

static void solution(const char *const input, char *const output, bool first) {
        struct problem problem;
        parse(input, &problem);

        #ifdef DEBUG
        for (size_t i=0; i<problem.recipes_count; i++) {
                fprintf(stderr, "Ingredients: ");
                for (size_t j=0; j<problem.recipes[i].ingredients.count; j++) {
                        char *ingredient = problem.recipes[i].ingredients.elements[j];
                        fprintf(stderr, "%s, ", ingredient);
                }
                fprintf(stderr, "\n");

                fprintf(stderr, "Allergens: ");
                for (size_t j=0; j<problem.recipes[i].allergens.count; j++) {
                        char *allergen = problem.recipes[i].allergens.elements[j];
                        fprintf(stderr, "%s, ", allergen);
                }
                fprintf(stderr, "\n-----\n");
        }
        fprintf(stderr, "\n");
        #endif

        struct set seen_allergens;
        set_init(&seen_allergens);

        struct set *allergen_ingredients;
        size_t allergen_ingredients_capacity = 8;
        size_t allergen_ingredients_count = 0;
        allergen_ingredients = malloc(allergen_ingredients_capacity * sizeof *allergen_ingredients);
        
        for (size_t i=0; i<problem.recipes_count; i++) {
                const struct recipe *const recipe = problem.recipes+i;
                for (size_t j=0; j<recipe->allergens.count; j++) {
                        char *allergen = recipe->allergens.elements[j];

                        size_t k;
                        if (!set_find_linear(&seen_allergens, allergen, &k)) {
                                set_add(&seen_allergens, strdup(allergen));

                                allergen_ingredients_count = k+1;
                                if (allergen_ingredients_count >= allergen_ingredients_capacity) {
                                        allergen_ingredients_capacity *= 2;
                                        allergen_ingredients = realloc(
                                                allergen_ingredients,
                                                allergen_ingredients_capacity * sizeof *allergen_ingredients);
                                }
                                set_init(allergen_ingredients+k);
                                set_copy_over(allergen_ingredients+k, &recipe->ingredients);
                        } else {
                                set_intersect(allergen_ingredients+k, &recipe->ingredients);
                        }
                }
        }

        #ifdef DEBUG
        for (size_t i=0; i<allergen_ingredients_count; i++) {
                fprintf(stderr, "%s: ", seen_allergens.elements[i]);
                for (size_t j=0; j<allergen_ingredients[i].count; j++) {
                        fprintf(stderr, "%s, ", allergen_ingredients[i].elements[j]);
                }
                fprintf(stderr, "\n");
        }
        fprintf(stderr, "\n");
        #endif

        if (first) {
                int total = 0;
                for (size_t i=0; i<problem.recipes_count; i++) {
                        const struct recipe *const recipe = problem.recipes+i;
                        for (size_t j=0; j<recipe->ingredients.count; j++) {
                                const char *const ingredient = recipe->ingredients.elements[j];
                                
                                bool all = true;
                                size_t k;
                                for (k=0; k<allergen_ingredients_count; k++) {
                                        size_t aux;
                                        if (set_find_binary(allergen_ingredients+k, ingredient, &aux)) {
                                                all = false;
                                                break;
                                        }
                                }
                                
                                if (all) {
                                        DBG("%s is not in any set", ingredient);
                                        total++;
                                } else {
                                        DBG("%s is in set %lu", ingredient, k);
                                }
                        }
                }
                
                snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
        } else {
                bool *decided_ingredients = malloc(allergen_ingredients_count * sizeof *allergen_ingredients);
                memset(decided_ingredients, 0, allergen_ingredients_count * sizeof *allergen_ingredients);
                bool all_ingredients_decided = false;

                struct pair *pairs = malloc(allergen_ingredients_count * sizeof *pairs);

                while (!all_ingredients_decided) {
                        all_ingredients_decided = true;
                        for (size_t i=0; i<allergen_ingredients_count; i++) {
                                ASSERT(allergen_ingredients[i].count > 0, "no candidates?");
                                if (!decided_ingredients[i]) {
                                        if (allergen_ingredients[i].count == 1) {
                                                decided_ingredients[i] = true;

                                                char *allergen = seen_allergens.elements[i];
                                                char *ingredient = allergen_ingredients[i].elements[0];
                                                pairs[i].allergen = strdup(allergen);
                                                pairs[i].ingredient = strdup(ingredient);
                                                DBG("%s's ingredient can only be %s",
                                                    allergen, ingredient);
                                                
                                                for (size_t j=0; j<allergen_ingredients_count; j++) {
                                                        if (i != j) {
                                                                set_remove(allergen_ingredients+j,
                                                                           allergen_ingredients[i].elements[0]);
                                                        }
                                                }
                                        } else {
                                                all_ingredients_decided = false;
                                        }
                                }
                        }
                }

                qsort(pairs, allergen_ingredients_count, sizeof *pairs, pair_cmp);

                char result[256];
                result[0] = '\0';
                size_t len = 0;
                for (size_t i=0; i<allergen_ingredients_count; i++) {
                        len += strlen(pairs[i].ingredient);
                        if (len > 255) {
                                FAIL("buffer overrun");
                        }
                        strcat(result, pairs[i].ingredient);
                        char *end = strchr(result, '\0');
                        *end = ',';
                        end++;
                        *end = '\0';
                }
                *(strchr(result, '\0')-1) = '\0';
                
                snprintf(output, OUTPUT_BUFFER_SIZE, "%s", result);
                
                free(decided_ingredients);
                for (size_t i=0; i<allergen_ingredients_count; i++) {
                        free(pairs[i].allergen);
                        free(pairs[i].ingredient);
                }
                free(pairs);
        }
        
        problem_free(&problem);
        for (size_t i=0; i<allergen_ingredients_count; i++) {
                set_free(allergen_ingredients+i);
        }
        free(allergen_ingredients);
        set_free(&seen_allergens);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, true);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, false);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
