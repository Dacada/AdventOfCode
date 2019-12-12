#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

struct vec3 {
        int x, y, z;
};

static int parse_vec3(const char *const input, struct vec3 *const v) {
        char coord = 'x';
        bool neg = false;
        for (int i=0; i<64; i++) {
                char c = input[i];
                if (c == '\n' || c == '\0') {
                        return i;
                } else if (isspace(c) || c == ',' || c == '<' || c == '>') {
                        continue;
                } else if (c == 'x' || c == 'y' || c == 'z') {
                        coord = c;
                } else if (c == '-') {
                        neg = true;
                } else if (c == '=' || (c >= '0' && c <= '9')) {
                        int *n;

                        if (coord == 'x') {
                                n = &v->x;
                        } else if (coord == 'y') {
                                n = &v->y;
                        } else {
                                n = &v->z;
                        }
                        
                        if (c == '=') {
                                *n = 0;
                                neg = false;
                        } else {
                                if (neg) {
                                        *n = -*n;
                                }
                                *n = *n * 10 + c - '0';
                                if (neg) {
                                        *n = -*n;
                                }
                        }
                } else {
                        FAIL("Unexpected character %c", c);
                }
        }
        FAIL("String too long");
}

static void parse_input(const char *const input, struct vec3 *vs, int n) {
        int i = 0;
        for (int j=0; j<n; j++) {
                i += parse_vec3(input+i, &vs[j]) + 1;
        }
}

static void apply_gravity_axis(int a, int b, int *va, int *vb) {
        if (a < b) {
                *va += 1;
                *vb -= 1;
        } else if (a > b) {
                *va -= 1;
                *vb += 1;
        }
}

static void apply_gravity_pair(int *arr, void *v_args) {
        struct {struct vec3 *vs; char axis;} *args = v_args;
        
        struct vec3 *a = args->vs+arr[0];
        struct vec3 *b = args->vs+arr[1];
        struct vec3 *va = a+4;
        struct vec3 *vb = b+4;

        if (args->axis == 'x') apply_gravity_axis(a->x, b->x, &va->x, &vb->x);
        if (args->axis == 'y') apply_gravity_axis(a->y, b->y, &va->y, &vb->y);
        if (args->axis == 'z') apply_gravity_axis(a->z, b->z, &va->z, &vb->z);
}

static void apply_gravity2(struct vec3 *vs, int n, char axis) {
        int arr[n];
        for (int i=0; i<n; i++) {
                arr[i] = i;
        }

        struct {struct vec3 *vs; char axis;} args = {.vs=vs, .axis=axis};
        aoc_combinations(arr, n, 2, apply_gravity_pair, &args);
}

static void apply_gravity(struct vec3 *vs, int n) {
        apply_gravity2(vs, n, 'x');
        apply_gravity2(vs, n, 'y');
        apply_gravity2(vs, n, 'z');
}

static void apply_velocity2(struct vec3 *vs, int n, char axis) {
        for (int i=0; i<n; i++) {
                if (axis == 'x') vs[i].x += vs[i+n].x;
                if (axis == 'y') vs[i].y += vs[i+n].y;
                if (axis == 'z') vs[i].z += vs[i+n].z;
        }
}

static void apply_velocity(struct vec3 *vs, int n) {
        apply_velocity2(vs, n, 'x');
        apply_velocity2(vs, n, 'y');
        apply_velocity2(vs, n, 'z');
}

static int energy(struct vec3 v) {
        return ABS(v.x) + ABS(v.y) + ABS(v.z);
}

static void solution1(const char *const input, char *const output) {
        struct vec3 moons[8];
        parse_input(input, moons, 4);
        for (int i=4; i<8; i++) {
                moons[i].x = moons[i].y = moons[i].z = 0;
        }

        const int timesteps = 1000;
        
        for (int t=0; t<timesteps; t++) {
                apply_gravity(moons, 4);
                apply_velocity(moons, 4);
        }

        int total = 0;
        for (int i=0; i<4; i++) {
                int pot = energy(moons[i]);
                int kin = energy(moons[i+4]);
                int tot = pot*kin;
                total += tot;
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

static long cycle_time(struct vec3 *vs, struct vec3 *ovs, int n, char axis) {
        long t;
        
        for (t=0;; t++) {
                apply_gravity2(vs, n, axis);
                apply_velocity2(vs, n, axis);

                bool eq = true;
                for (int i=0; i<n; i++) {
                        int current, original;
                        int currentv, originalv;

                        if (axis == 'x') {
                                current = vs[i].x;
                                currentv = vs[i+4].x;
                                original = ovs[i].x;
                                originalv = ovs[i+4].x;
                        } else if (axis == 'y') {
                                current = vs[i].y;
                                currentv = vs[i+4].y;
                                original = ovs[i].y;
                                originalv = ovs[i+4].y;
                        } else if (axis == 'z') {
                                current = vs[i].z;
                                currentv = vs[i+4].z;
                                original = ovs[i].z;
                                originalv = ovs[i+4].z;
                        } else {
                                FAIL("unexpected axis");
                        }
                        
                        if (original != current || originalv != currentv) {
                                eq = false;
                                break;
                        }
                }
                if (eq) {
                        break;
                }
        }

        return t+1;
}

static long lgcd(long a, long b) {
        int remainder = 0;
        
        do {
                remainder = a % b;
                a = b; 
                b = remainder;
        } while (b != 0);
        
        return a;
}

static long llcm2(long a, long b) {
        if (a == 0 || b == 0) {
                return 0;
        }
        return a*b/lgcd(a,b);
}

static long llcm3(long a, long b, long c) {
        return llcm2(llcm2(a,b),c);
}

static void solution2(const char *const input, char *const output) {
        struct vec3 moons[8];
        parse_input(input, moons, 4);
        for (int i=4; i<8; i++) {
                moons[i].x = moons[i].y = moons[i].z = 0;
        }

        struct vec3 original_moons[8];
        for (int i=0; i<8; i++) {
                original_moons[i] = moons[i];
        }
        
        long tx = cycle_time(moons, original_moons, 4, 'x');
        long ty = cycle_time(moons, original_moons, 4, 'y');
        long tz = cycle_time(moons, original_moons, 4, 'z');

        long result = llcm3(tx, ty, tz);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
