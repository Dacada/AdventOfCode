#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define IDX(cube,dim) ((cube).x + (cube).y*(dim) + (cube).z*(dim)*(dim))
#define STACK_SIZE (1<<14)

struct point {
        int x;
        int y;
        int z;
};

static int parse_int(const char **input) {
        ASSERT(isdigit(**input), "parse error");
        int r = 0;
        while (isdigit(**input)) {
                r *= 10;
                r += **input - '0';
                *input += 1;
        }
        return r;
}

static void parse_line(const char **input, struct point *cube) {
        cube->x = parse_int(input);
        ASSERT(**input == ',', "parse error");
        *input += 1;
        cube->y = parse_int(input);
        ASSERT(**input == ',', "parse error");
        *input += 1;
        cube->z = parse_int(input);
}

static void skip_newlines(const char **input) {
        while (**input == '\n') {
                *input += 1;
        }
}

static int parse_input(const char *input, struct point **cubes) {
        int cap = 16;
        int len = 0;
        *cubes = malloc(sizeof(**cubes)*cap);

        while (*input != '\0') {
                if (len >= cap) {
                        cap *= 2;
                        *cubes = realloc(*cubes, sizeof(**cubes)*cap);
                }
                parse_line(&input, (*cubes)+len);
                len++;
                skip_newlines(&input);
        }

        return len;
}

// In order for two cubes to share a face, they need to share two axis and have
// a difference of 1 unit for the other.
static bool adjacent(const struct point *cube1, const struct point *cube2) {
        int dx = abs(cube1->x - cube2->x);
        int dy = abs(cube1->y - cube2->y);
        int dz = abs(cube1->z - cube2->z);

        ASSERT(!(dx==0 && dy==0 && dz==0), "identical cubes?");
        
        return
                (dx == 0 && dy == 0 && dz == 1) ||
                (dx == 0 && dy == 1 && dz == 0) ||
                (dx == 1 && dy == 0 && dz == 0);
}

static void solution1(const char *const input, char *const output) {
        struct point *cubes;
        int len = parse_input(input, &cubes);

        int *faces = malloc(sizeof(*faces)*len);
        for (int i=0; i<len; i++) {
                faces[i] = 6;
        }

        for (int i=0; i<len; i++) {
                for (int j=i+1; j<len; j++) {
                        if (adjacent(&cubes[i], &cubes[j])) {
                                faces[i]--;
                                faces[j]--;
                        }
                }
        }

        int total = 0;
        for (int i=0; i<len; i++) {
                ASSERT(faces[i]>=0, "invalid number of faces!");
                total += faces[i];
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
        free(cubes);
        free(faces);
}

static int get_area_dimensions(struct point *cubes, int len) {
        int max = 0;
        int min = 999;
        for (int i=0; i<len; i++) {
                if (cubes[i].x > max) {
                        max = cubes[i].x;
                }
                if (cubes[i].y > max) {
                        max = cubes[i].y;
                }
                if (cubes[i].z > max) {
                        max = cubes[i].z;
                }
                if (cubes[i].x < min) {
                        min = cubes[i].x;
                }
                if (cubes[i].y > min) {
                        min = cubes[i].y;
                }
                if (cubes[i].z > min) {
                        min = cubes[i].z;
                }
        }
        ASSERT(max > 0, "impossible cube positions");
        ASSERT(min >= 1, "there should be no cubes at 0 or lower");
        (void)min;

        return max + 2;
}

static int search(const bool *cube_map, int dim) {
        static const struct point cardinals[] = {
                {.x=0,  .y=0,  .z=1 },
                {.x=0,  .y=0,  .z=-1},
                {.x=0,  .y=1,  .z=0 },
                {.x=0,  .y=-1, .z=0 },
                {.x=1,  .y=0,  .z=0 },
                {.x=-1, .y=0,  .z=0 },
        };

        bool *visited = malloc(sizeof(*visited)*dim*dim*dim);
        for (int i=0; i<dim*dim*dim; i++) {
                visited[i] = false;
        }
        
        static struct point stack[STACK_SIZE];
        int stack_len = 0;

        stack[0].x = 0;
        stack[0].y = 0;
        stack[0].z = 0;
        stack_len = 1;

        int total = 0;
        while (stack_len > 0) {
                struct point p = stack[--stack_len];

                if (visited[IDX(p,dim)]) {
                        continue;
                }
                visited[IDX(p,dim)] = true;

                for (unsigned i=0; i<sizeof(cardinals)/sizeof(*cardinals); i++) {
                        struct point newp = p;
                        newp.x += cardinals[i].x;
                        newp.y += cardinals[i].y;
                        newp.z += cardinals[i].z;

                        if (newp.x < 0 || newp.x >= dim ||
                            newp.y < 0 || newp.y >= dim ||
                            newp.z < 0 || newp.z >= dim) {
                                continue;
                        }

                        if (visited[IDX(newp,dim)]) {
                                continue;
                        }

                        if (cube_map[IDX(newp,dim)]) {
                                if (newp.x==3 && newp.y==3 && newp.z==2)
                                        DBG("collided with cube %d,%d,%d from %d,%d,%d by adding %d,%d,%d", newp.x,newp.y,newp.z, p.x,p.y,p.z, cardinals[i].x,cardinals[i].y,cardinals[i].z);
                                total++;
                                continue;
                        }

                        stack[stack_len++] = newp;
                }
        }

        free(visited);
        return total;
}

static void solution2(const char *const input, char *const output) {
        struct point *cubes;
        int len = parse_input(input, &cubes);

        // raise cubes by 1 on each axis to ensure there's nothing on 0
        for (int i=0; i<len; i++) {
                cubes[i].x++;
                cubes[i].y++;
                cubes[i].z++;
        }

        int dim = get_area_dimensions(cubes, len);
        bool *cube_map = malloc(sizeof(*cube_map)*dim*dim*dim);
	memset(cube_map, 0, sizeof(*cube_map)*dim*dim*dim);
        for (int i=0; i<len; i++) {
                cube_map[IDX(cubes[i],dim)] = true;
        }
        free(cubes);

        int res = search(cube_map, dim);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
        free(cube_map);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
