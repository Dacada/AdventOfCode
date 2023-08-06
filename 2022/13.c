#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

#define MAX(a,b) ((a)>(b)?(a):(b))

enum type {
        TYPE_INTEGER,
        TYPE_LIST,
};

struct list {
        int len;
        struct element *elements;
};

struct element {
        enum type type;
        union {
                int integer;
                struct list list;
        };
};

#ifdef DEBUG
static void list_prnt(const struct list *l);
static void element_prnt(const struct element *e) {
        if (e->type == TYPE_LIST) {
                list_prnt(&e->list);
        } else {
                fprintf(stderr, "%d", e->integer);
        }
}

static void list_prnt(const struct list *l) {
        fputc('[', stderr);
        for (int i=0; i<l->len; i++) {
                element_prnt(&l->elements[i]);
                fputc(',', stderr);
        }
        fputc(']', stderr);
}
#endif

static void list_free(struct list *l) {
        for (int i=0; i<l->len; i++) {
                if (l->elements[i].type == TYPE_LIST) {
                        list_free(&l->elements[i].list);
                }
        }
        free(l->elements);
}

static int element_cmp(const struct element *left, const struct element *right);

__attribute__((pure))
static int list_cmp(const struct list *left, const struct list *right) {
        int len = MAX(left->len, right->len);
        for (int i=0; i<len; i++) {
                if (i >= left->len) {
                        return -1;
                }
                if (i >= right->len) {
                        return 1;
                }
                
                int ret = element_cmp(&left->elements[i], &right->elements[i]);
                if (ret != 0) {
                        return ret;
                }
        }

        return 0;
}

static int int_cmp(int left, int right) {
        return left - right;
}

static int element_cmp(const struct element *left, const struct element *right) {
        if (left->type != right->type) {
                struct element dummyelements[1];
                dummyelements[0].type = TYPE_INTEGER;
                struct list dummy;
                dummy.len = 1;
                dummy.elements = dummyelements;
                if (left->type == TYPE_INTEGER) {
                        dummyelements[0].integer = left->integer;
                        return list_cmp(&dummy, &right->list);
                } else {
                        dummyelements[0].integer = right->integer;
                        return list_cmp(&left->list, &dummy);
                }
        }
        
        switch (left->type) {
        case TYPE_INTEGER:
                return int_cmp(left->integer, right->integer);
        case TYPE_LIST:
                return list_cmp(&left->list, &right->list);
        default:
                FAIL("invalid type");
        }
}

static void parse_element(const char **input, struct element *e);
static void parse_list(const char **input, struct list *list) {
        ASSERT(**input == '[', "parse error %c", **input);
        *input += 1;

        list->len = 0;
        int cap = 8;
        list->elements = malloc(sizeof(*list->elements) * cap);

        while (**input != ']') {
                if (list->len >= cap) {
                        cap *= 2;
                        list->elements = realloc(list->elements, sizeof(*list->elements) * cap);
                }
                
                parse_element(input, &list->elements[list->len++]);
                
                if (**input == ',') {
                        *input += 1;
                } else {
                        ASSERT(**input == ']', "parse error");
                }
        }
        *input += 1;
}

static int parse_int(const char **input) {
        int r = 0;
        ASSERT(isdigit(**input), "parse error");
        while (isdigit(**input)) {
                r *= 10;
                r += **input - '0';
                *input += 1;
        }
        return r;
}

static void parse_element(const char **input, struct element *e) {
        if (**input == '[') {
                e->type = TYPE_LIST;
                parse_list(input, &e->list);
        } else {
                e->type = TYPE_INTEGER;
                e->integer = parse_int(input);
        }
}

static int parse_input(const char *input, struct list **pairs) {
        int len = 0;
        int cap = 8;
        *pairs = malloc(sizeof(**pairs)*cap);
        while (*input != '\0') {
                if (len+3 >= cap) { // space to accomodate a second element plus two extra for part 2
                        cap *= 2;
                        *pairs = realloc(*pairs, sizeof(**pairs)*cap);
                }
                
                parse_list(&input, &(*pairs)[len++]);
                ASSERT(*input == '\n', "parse error");
                input += 1;
                
                parse_list(&input, &(*pairs)[len++]);
                ASSERT(*input == '\n' || *input == '\0', "parse error");
                while (*input == '\n') {
                        input += 1;
                }
        }

        return len;
}

static void solution1(const char *const input, char *const output) {
        struct list *pairs;
        int len = parse_input(input, &pairs);

        int total = 0;
        for (int i=0; i<len/2; i++) {
                DBG("pair %d", i);
                if (list_cmp(&pairs[i*2], &pairs[i*2+1]) < 0) {
                        DBG("CORRECT");
                        total += i+1;
                } else {
                        DBG("WRONG");
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
        for (int i=0; i<len; i++) {
                list_free(&pairs[i]);
        }
        free(pairs);
}

static void *init_divider(struct list *list, int n) {
        void *ptr;
        list->len = 1;
        list->elements = ptr = malloc(sizeof(struct element));
        list->elements[0].type = TYPE_LIST;
        list->elements[0].list.len = 1;
        list->elements[0].list.elements = malloc(sizeof(struct element));
        list->elements[0].list.elements[0].type = TYPE_INTEGER;
        list->elements[0].list.elements[0].integer = n;
        return ptr;
}

__attribute__((pure))
static int list_cmp_wrp(const void *a, const void *b) {
        return list_cmp(a, b);
}

static void solution2(const char *const input, char *const output) {
        struct list *lists;
        int len = parse_input(input, &lists);
        
        void *ptr1 = init_divider(lists + len, 2);
        void *ptr2 = init_divider(lists + len + 1, 6);
        len += 2;

        qsort(lists, len, sizeof(*lists), list_cmp_wrp);
        int result = 1;
        for (int i=0; i<len; i++) {
#ifdef DEBUG
                list_prnt(&lists[i]);
                fputc('\n', stderr);
#endif
                if (lists[i].elements == ptr1 || lists[i].elements == ptr2) {
                        result *= i + 1;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
        for (int i=0; i<len; i++) {
                list_free(&lists[i]);
        }
        free(lists);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
