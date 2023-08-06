#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct vertex {
        int x, y, z;
};

struct cuboid {
        struct vertex min;
        struct vertex max;
};

struct instruction {
        struct cuboid cuboid;
        bool on;
};

struct cubeCollection {
        size_t size;
        size_t len;
        struct cuboid *cubes;
};

static void cubeCollection_realloc(struct cubeCollection *const cubes) {
        cubes->cubes = realloc(cubes->cubes, sizeof(*cubes->cubes) * cubes->size);
}

static void cubeCollection_init(struct cubeCollection *const cubes) {
        cubes->size = 2;
        cubes->len = 0;
        cubes->cubes = NULL;
        cubeCollection_realloc(cubes);
}

static void cubeCollection_free(struct cubeCollection *const cubes) {
        free(cubes->cubes);
}

static struct cuboid *cubeCollection_add(struct cubeCollection *const cubes) {
        if (cubes->len >= cubes->size) {
                cubes->size *= 2;
                cubeCollection_realloc(cubes);
        }
        struct cuboid *ret = cubes->cubes + cubes->len;
        cubes->len++;
        return ret;
}

static int parse_number(const char **const input) {
        bool neg = false;
        if (**input == '-') {
                neg = true;
                *input += 1;
        }

        ASSERT(isdigit(**input), "parse error");
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

static void assert_msg(const char **const input, const char *const msg) {
        size_t len = strlen(msg);
        ASSERT(strncmp(*input, msg, len) == 0, "parse error");
        *input += len;
}

static void parse_instruction(const char **const input, struct instruction *const instr) {
        ASSERT(**input == 'o', "parse error");
        *input += 1;
        
        if (**input == 'n') {
                instr->on = true;
                *input += 2;
        } else if (**input == 'f') {
                instr->on = false;
                *input += 3;
        } else {
                FAIL("parse error");
        }

        assert_msg(input, "x=");
        instr->cuboid.min.x = parse_number(input);

        assert_msg(input, "..");
        instr->cuboid.max.x = parse_number(input);

        assert_msg(input, ",y=");
        instr->cuboid.min.y = parse_number(input);

        assert_msg(input, "..");
        instr->cuboid.max.y = parse_number(input);

        assert_msg(input, ",z=");
        instr->cuboid.min.z = parse_number(input);

        assert_msg(input, "..");
        instr->cuboid.max.z = parse_number(input);

        ASSERT(**input == '\n', "parse error");
        *input += 1;
}

static struct instruction *parse_input(const char *input, size_t *const len) {
        size_t size = 32;
        *len = 0;
        struct instruction *list = malloc(size*sizeof(*list));

        while (*input != '\0') {
                if (*len >= size) {
                        size *= 2;
                        list = realloc(list, size*sizeof(*list));
                }
                parse_instruction(&input, list + *len);
                *len += 1;
        }

        return list;
}

static unsigned long volume_cuboid(const struct cuboid *const cube) {
        // Plus 1 since both max and min are included.
        long dx = cube->max.x - cube->min.x + 1;
        long dy = cube->max.y - cube->min.y + 1;
        long dz = cube->max.z - cube->min.z + 1;
        
        ASSERT(dx>0 && dy>0 && dz>0, "invalid cuboid max=(%d,%d,%d) min=(%d,%d,%d)", cube->max.x, cube->max.y, cube->max.z, cube->min.x, cube->min.y, cube->min.z);
        return dx*dy*dz;
}

__attribute__((pure))
static unsigned long total_volume(const struct cubeCollection *const cubes) {
        long res = 0;

        for (size_t i=0; i<cubes->len; i++) {
                res += volume_cuboid(&cubes->cubes[i]);
        }
        
        return res;
}

// The optimizer will hopefully optimize much of the trash in this function
static inline bool cuboid_collision_axis(const struct cuboid *const a, const struct cuboid *const b,
                                         struct cubeCollection *const newCubes, const size_t axis) {
#define cV(x) *((const int*)((const char*)&(x)+axis))
#define V(x) *((int*)((char*)&(x)+axis))
#define SET_X                                       \
        struct cuboid *x = newCubes->cubes + i
#define SET_XY                                                  \
        struct cuboid *y = cubeCollection_add(newCubes);        \
        struct cuboid *x = newCubes->cubes + i;                 \
        *y = *x
#define SET_XYZ                                                 \
        cubeCollection_add(newCubes);                           \
        struct cuboid *z = cubeCollection_add(newCubes);        \
        struct cuboid *y = newCubes->cubes+newCubes->len-2;     \
        struct cuboid *x = newCubes->cubes + i;                 \
        *y = *x;                                                \
        *z = *x
#define VALIDATE(a)                                             \
        ASSERT(cV(a->min) <= cV(a->max), "invalid cuboid")
        
        size_t l = newCubes->len;
        for (size_t i=0; i<l; i++) {
                
                // Amin       Amax  Bmin       Bmax
                // ~~~~~~~~~~~~     ------------
                if (cV(a->min) < cV(b->min) &&
                    cV(a->min) < cV(b->max) &&
                    cV(a->max) < cV(b->min) &&
                    cV(a->max) < cV(b->max)) {
                        return false;
                }
        
                // Bmin       Bmax  Amin       Amax
                // ------------     ~~~~~~~~~~~~
                else if (cV(a->min) > cV(b->min) &&
                         cV(a->min) > cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) > cV(b->max)) {
                        return false;
                }
        
                // Amin  Bmin       Amax  Bmax
                // ~~~~~~============------
                //       XXXXXXXXXXXXYYYYYY
                else if (cV(a->min) < cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) < cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(a->max);
                        VALIDATE(x);

                        V(y->min) = cV(a->max)+1;
                        V(y->max) = cV(b->max);
                        VALIDATE(y);
                }

                // Bmin  Amin       Bmax  Amax
                // ------============~~~~~~
                // XXXXXXYYYYYYYYYYYY
                else if (cV(a->min) > cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) > cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(a->min)-1;
                        VALIDATE(x);

                        V(y->min) = cV(a->min);
                        V(y->max) = cV(b->max);
                        VALIDATE(y);
                }

                // Amin  Bmin       Bmax  Amax
                // ~~~~~~============~~~~~~
                //       XXXXXXXXXXXX
                else if (cV(a->min) < cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) > cV(b->max)) {
                        SET_X;
                        
                        V(x->min) = cV(b->min);
                        V(x->max) = cV(b->max);
                        VALIDATE(x);
                }
                
                // Bmin  Amin       Amax  Bmax
                // ------============------
                // XXXXXXYYYYYYYYYYYYZZZZZZ
                else if (cV(a->min) > cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) < cV(b->max)) {
                        SET_XYZ;
                        
                        V(x->min) = cV(b->min);
                        V(x->max) = cV(a->min)-1;
                        VALIDATE(x);

                        V(y->min) = cV(a->min);
                        V(y->max) = cV(a->max);
                        VALIDATE(y);

                        V(z->min) = cV(a->max)+1;
                        V(z->max) = cV(b->max);
                        VALIDATE(z);
                }
        
                // Amin
                // Bmin       Amax  Bmax
                // ============------
                // XXXXXXXXXXXXYYYYYY
                else if (cV(a->min) == cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) < cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(a->max);
                        VALIDATE(x);
                        
                        V(y->min) = cV(a->max)+1;
                        V(y->max) = cV(b->max);
                        VALIDATE(y);
                }

                // Amin
                // Bmin       Bmax  Amax
                // ============------
                // XXXXXXXXXXXX
                else if (cV(a->min) == cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) > cV(b->max)) {
                        SET_X;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(b->max);
                }
                
                // Amin  Bmin       Amax
                //                  Bmax
                // ~~~~~~============
                //       XXXXXXXXXXXX
                else if (cV(a->min) < cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) == cV(b->max)) {
                        SET_X;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(b->max);
                        VALIDATE(x);
                }
                
                // Bmin  Amin       Amax
                //                  Bmax
                // ~~~~~~============
                // XXXXXXYYYYYYYYYYYY
                else if (cV(a->min) > cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) == cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(a->min)-1;
                        V(y->min) = cV(a->min);
                        V(y->max) = cV(b->max);
                }
                
                // Amin       Amax
                // Bmin       Bmax
                // ============
                // XXXXXXXXXXXX
                else if (cV(a->min) == cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) == cV(b->max)) {
                        SET_X;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(b->max);
                        VALIDATE(x);
                }
                
                // Amin Amax
                //      Bmin  Bmax
                // -----=------
                //      XYYYYYY
                else if (cV(a->min) < cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) == cV(b->min) &&
                         cV(a->max) < cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(a->max);

                        V(y->min) = cV(a->max)+1;
                        V(y->max) = cV(b->max);
                }
                
                // Bmin Bmax
                //      Amin  Amax
                // -----=------
                // XXXXXY
                else if (cV(a->min) > cV(b->min) &&
                         cV(a->min) == cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) > cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(a->min)-1;

                        V(y->min) = cV(b->max);
                        V(y->max) = cV(a->min);
                }
                
                // Amin
                // Amax
                // Bmin  Bmax
                // =------
                // XYYYYYY
                else if (cV(a->min) == cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) == cV(b->min) &&
                         cV(a->max) < cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->min) = cV(a->min);

                        V(y->min) = cV(a->min)+1;
                        V(y->max) = cV(b->max);
                }
                
                // Bmin
                // Bmax
                // Amin  Amax
                // =------
                // X
                else if (cV(a->min) == cV(b->min) &&
                         cV(a->min) == cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) > cV(b->max)) {
                        SET_X;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(b->max);
                }
                
                //       Amin
                //       Amax
                // Bmin  Bmax
                // ------=
                // XXXXXXY
                else if (cV(a->min) > cV(b->min) &&
                         cV(a->min) == cV(b->max) &&
                         cV(a->max) > cV(b->min) &&
                         cV(a->max) == cV(b->max)) {
                        SET_XY;

                        V(x->min) = cV(b->min);
                        V(x->min) = cV(a->min)-1;

                        V(y->min) = cV(a->min);
                        V(y->max) = cV(b->max);
                }
                
                //       Bmin
                //       Bmax
                // Amin  Amax
                // ------=
                //       X
                else if (cV(a->min) < cV(b->min) &&
                         cV(a->min) < cV(b->max) &&
                         cV(a->max) == cV(b->min) &&
                         cV(a->max) == cV(b->max)) {
                        SET_X;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(b->max);
                }
                
                // Amin
                // Amax
                // Bmin
                // Bmax
                // =
                // X
                else if (cV(a->min) == cV(b->min) &&
                         cV(a->min) == cV(b->max) &&
                         cV(a->max) == cV(b->min) &&
                         cV(a->max) == cV(b->max)) {
                        SET_X;

                        V(x->min) = cV(b->min);
                        V(x->max) = cV(b->max);
                }

                else {
#ifdef DEBUG
                        int dim_a_x = a->max.x - a->min.x;
                        int dim_a_y = a->max.y - a->min.y;
                        int dim_a_z = a->max.z - a->min.z;
                        int dim_b_x = b->max.x - b->min.x;
                        int dim_b_y = b->max.y - b->min.y;
                        int dim_b_z = b->max.z - b->min.z;
                        FAIL("impossible? (axis=%lu)\na: min=(%d,%d,%d) dimensions=(%d,%d,%d)\nb: min=(%d,%d,%d) dimensions=(%d,%d,%d)", axis, a->min.x, a->min.y, a->min.z, dim_a_x, dim_a_y, dim_a_z, b->min.x, b->min.y, b->min.z, dim_b_x, dim_b_y, dim_b_z);
#endif
                }
        }

        return true;
#undef cV
#undef V
#undef SET_X
#undef SET_XY
#undef SET_XYZ
#undef VALIDATE
}

// Return whether a and b collide, create 6 new cubes which divide b into
// several constituent pieces that a doesn't intersect with.
static inline bool cuboid_collision(const struct cuboid *const a, const struct cuboid *const b, struct cubeCollection *newCubes) {

        cubeCollection_add(newCubes);
        if (!cuboid_collision_axis(a, b, newCubes, offsetof(struct vertex, x))) {
                return false;
        }
        if (!cuboid_collision_axis(a, b, newCubes, offsetof(struct vertex, y))) {
                return false;
        }
        if (!cuboid_collision_axis(a, b, newCubes, offsetof(struct vertex, z))) {
                return false;
        }

        // One of the cuboids we collected is completely contained within A
        // while the rest don't collide with A at all. This extra cuboid would
        // have sections that don't collide with A in higher dimensions but who
        // cares about those. Remove it (assert it exists) and also assert that
        // all cuboids are valid.
        size_t remove = 0;
        bool found = false;
        for (size_t i=0; i<newCubes->len; i++) {
                struct cuboid *c = &newCubes->cubes[i];
                if (c->min.x >= a->min.x && c->min.x <= a->max.x &&
                    c->max.x >= a->min.x && c->max.x <= a->max.x &&
                    c->min.y >= a->min.y && c->min.y <= a->max.y &&
                    c->max.y >= a->min.y && c->max.y <= a->max.y &&
                    c->min.z >= a->min.z && c->min.z <= a->max.z &&
                    c->max.z >= a->min.z && c->max.z <= a->max.z) {
                        ASSERT(!found, "two cubes contained within A?");
                        found = true;
                        remove = i;
                }

                // Also assert all new cuboids are valid
                ASSERT(c->min.x <= c->max.x && c->min.y <= c->max.y && c->min.z <= c->max.z, "invalid cuboid created");
        }
        ASSERT(found, "no cubes contained within A?");
        newCubes->cubes[remove] = newCubes->cubes[--newCubes->len];

        return true;
}

/* // a is completely enclosed inside of b (or they're identical) */
/* static inline bool completely_enclose(const struct cuboid *const a, const struct cuboid *const b) { */
/*         return a->min.x <= b->min.x && a->min.x <= b->max.x && */
/*                 a->max.x <= b->min.x && a->max.x <= b->max.x && */
/*                 a->min.y <= b->min.y && a->min.y <= b->max.y && */
/*                 a->max.y <= b->min.y && a->max.y <= b->max.y && */
/*                 a->min.z <= b->min.z && a->min.z <= b->max.z && */
/*                 a->max.z <= b->min.z && a->max.z <= b->max.z; */
/* } */

static struct cubeCollection *handle_cube(struct cubeCollection *const cubes, const struct cuboid *cube, bool add) {
        struct cuboid *c;
        
        struct cubeCollection *newCubes = malloc(sizeof(*newCubes));
        cubeCollection_init(newCubes);
        
        for (size_t i=0; i<cubes->len; i++) {
                struct cuboid *other = &cubes->cubes[i];
                
                struct cubeCollection otherPieces;
                cubeCollection_init(&otherPieces);
                if (cuboid_collision(cube, other, &otherPieces)) {
                        // Cut up other into pieces that don't
                        // intersect with cube and add those.
                        for (size_t j=0; j<otherPieces.len; j++) {
                                c = cubeCollection_add(newCubes);
                                *c = otherPieces.cubes[j];
                        }
                        // If other is add we will add it later, if it's
                        // subtract we just don't do anything later and
                        // it will have subtracted from other
                } else {
                        // No collision, add other back to the new set
                        c = cubeCollection_add(newCubes);
                        *c = *other;
                        // We deal with cube later
                }
                cubeCollection_free(&otherPieces);
        }

        // Checked all possible collisions, now we either add cube or we don't.
        if (add) {
                c = cubeCollection_add(newCubes);
                *c = *cube;
        }

        cubeCollection_free(cubes);
        free(cubes);
        return newCubes;
}

static void solution(const char *const input, char *const output, bool all) {
        size_t ninstructions;
        struct instruction *instructions = parse_input(input, &ninstructions);

        struct cubeCollection *cubes = malloc(sizeof(*cubes));
        cubeCollection_init(cubes);

        for (size_t i=0; i<ninstructions; i++) {
                struct instruction *instr = instructions + i;
                if (all || (instr->cuboid.min.x <= 50 && instr->cuboid.min.x >= -50)) {
                        cubes = handle_cube(cubes, &instr->cuboid, instr->on);
                }
        }

        unsigned long result = total_volume(cubes);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
        free(instructions);
        cubeCollection_free(cubes);
        free(cubes);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, false);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, true);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
