#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

struct number {
        struct number *parent;
        bool leaf;
        union {
                unsigned n;
                struct {
                        struct number *x;
                        struct number *y;
                } children;
        } value;
};

static void number_free(struct number *const number) {
        if (number->leaf) {
                return;
        }
        
        number_free(number->value.children.x);
        number_free(number->value.children.y);

        free(number->value.children.x);
        free(number->value.children.y);
}

static struct number *number_sum(struct number *const first, struct number *const second) {
        struct number *res = malloc(sizeof(*res));
        res->leaf = false;
        res->value.children.x = first;
        res->value.children.y = second;
        res->parent = NULL;
        first->parent = res;
        second->parent = res;
        return res;
}

static struct number *find_explode(struct number *const number, unsigned current_depth) {
        ASSERT(current_depth <= 4, "pairs too deeply neseted");

        if (number->leaf) {
                return NULL;
        }
        
        if (current_depth == 4) {
                return number;
        } else {
                struct number *ret = find_explode(number->value.children.x, current_depth + 1);
                if (ret != NULL) {
                        return ret;
                }
                return find_explode(number->value.children.y, current_depth + 1);
        }
}

#define find_x_from(number, member, other, function)                    \
        if (number->parent == NULL) {                                   \
                return NULL;                                            \
        }                                                               \
                                                                        \
        struct number *n = number->parent->value.children.member;       \
        if (n == number) {                                              \
                return function(number->parent);                        \
        } else {                                                        \
                while (!n->leaf) {                                      \
                        n = n->value.children.other;                    \
                }                                                       \
                return n;                                               \
        }

static struct number *find_left_from(struct number *const number) {
        find_x_from(number, x, y, find_left_from);
}

static struct number *find_right_from(struct number *const number) {
        find_x_from(number, y, x, find_right_from);
}

static bool explode(struct number *const number) {
        struct number *const pair = find_explode(number, 0);
        if (pair == NULL) {
                return false;
        }
        
        ASSERT(!pair->leaf, "invalid find_explode");
        ASSERT(pair->value.children.x->leaf, "invalid number");
        ASSERT(pair->value.children.y->leaf, "invalid number");

        unsigned x = pair->value.children.x->value.n;
        unsigned y = pair->value.children.y->value.n;

        struct number *const left = find_left_from(pair);
        if (left != NULL) {
                ASSERT(left->leaf, "invalid find_left");
                left->value.n += x;
        }

        struct number *const right = find_right_from(pair);
        if (right != NULL) {
                ASSERT(right->leaf, "invalid find_righ");
                right->value.n += y;
        }

        free(pair->value.children.x);
        free(pair->value.children.y);
        pair->leaf = true;
        pair->value.n = 0;

        return true;
}

static struct number *find_split(struct number *const number) {
        if (number->leaf) {
                if (number->value.n >= 10) {
                        return number;
                } else {
                        return NULL;
                }
        } else {
                struct number *ret = find_split(number->value.children.x);
                if (ret != NULL) {
                        return ret;
                }
                return find_split(number->value.children.y);
        }
}

static bool split(struct number *const number) {
        struct number *const n = find_split(number);
        if (n == NULL) {
                return false;
        }

        ASSERT(n->leaf, "invalid find_split");

        unsigned x = n->value.n / 2;
        unsigned y = n->value.n / 2 + n->value.n % 2;

        struct number *new_x = malloc(sizeof(*new_x));
        struct number *new_y = malloc(sizeof(*new_y));

        new_x->parent = n;
        new_y->parent = n;
        new_x->leaf = true;
        new_y->leaf = true;
        new_x->value.n = x;
        new_y->value.n = y;

        n->leaf = false;
        n->value.children.x = new_x;
        n->value.children.y = new_y;

        return true;
}

static void reduce(struct number *const number) {
        for (;;) {
                if (explode(number)) {
                        continue;
                }
                if (!split(number)) {
                        break;
                }
        }
}

static void parse_number(const char **const input, struct number *root) {
        if (**input == '[') {
                *input += 1;
                root->leaf = false;

                root->value.children.x = malloc(sizeof(*root));
                parse_number(input, root->value.children.x);
                root->value.children.x->parent = root;
                
                ASSERT(**input == ',', "parse error %s", *input);
                *input += 1;
                
                root->value.children.y = malloc(sizeof(*root));
                parse_number(input, root->value.children.y);
                root->value.children.y->parent = root;

                ASSERT(**input == ']', "parse error");
                *input += 1;
        } else if (isdigit(**input)) {
                root->leaf = true;
                ASSERT(isdigit(**input), "parse error");
                root->value.n = **input - '0';
                *input += 1;
        } else {
                FAIL("parse error");
        }
        
        root->parent = NULL;
}

static struct number **parse_input(const char *input, size_t *const len) {
        size_t size = 8;
        *len = 0;
        struct number **list = malloc(size * sizeof(*list));

        while (*input != '\0') {
                if (*len >= size) {
                        size *= 2;
                        list = realloc(list, size * sizeof(*list));
                }
                list[*len] = malloc(sizeof(struct number));
                parse_number(&input, list[*len]);
                *len += 1;

                ASSERT(*input == '\n' || *input == '\0', "parse error");
                if (*input == '\n') {
                        input += 1;
                }
        }

        return list;
}

static unsigned magnitude(const struct number *const number) {
        if (number->leaf) {
                return number->value.n;
        } else {
                return 3*magnitude(number->value.children.x) + 2*magnitude(number->value.children.y);
        }
}

static void solution1(const char *const input, char *const output) {
        size_t len_numbers;
        struct number **numbers = parse_input(input, &len_numbers);
        ASSERT(len_numbers>0, "parse error");

        struct number *total = numbers[0];
        for (size_t i=1; i<len_numbers; i++) {
                total = number_sum(total, numbers[i]);
                reduce(total);
        }
        free(numbers);

        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", magnitude(total));
        number_free(total);
        free(total);
}

static struct number *copy_number(const struct number *const number) {
        struct number *copy = malloc(sizeof(*copy));
        copy->leaf = number->leaf;
        copy->value = number->value;
        
        if (!copy->leaf) {
                copy->value.children.x = copy_number(number->value.children.x);
                copy->value.children.y = copy_number(number->value.children.y);
                copy->value.children.x->parent = copy;
                copy->value.children.y->parent = copy;
        }

        copy->parent = NULL;
        return copy;
}

static void solution2(const char *const input, char *const output) {
        size_t len_numbers;
        struct number **numbers = parse_input(input, &len_numbers);

        unsigned max = 0;
        for (size_t i=0; i<len_numbers; i++) {
                for (size_t j=0; j<len_numbers; j++) {
                        if (i == j) {
                                continue;
                        }

                        struct number *n1 = copy_number(numbers[i]);
                        struct number *n2 = copy_number(numbers[j]);
                        
                        struct number *r = number_sum(n1, n2);
                        reduce(r);
                        
                        unsigned m = magnitude(r);
                        
                        number_free(r);
                        free(r);

                        if (m > max) {
                                max = m;
                        }
                }
        }

        for (size_t i=0; i<len_numbers; i++) {
                number_free(numbers[i]);
                free(numbers[i]);
        }
        free(numbers);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", max);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
