#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* static int mod(int a, int b) { */
/*         int r = a % b; */
/*         return r < 0 ? r + b : r; */
/* } */

static int parse_int(const char **input) {
        bool neg = **input == '-';
        if (neg) {
                *input += 1;
        }
        ASSERT(isdigit(**input), "parse error");
        int r = 0;
        while (isdigit(**input)) {
                r *= 10;
                r += **input - '0';
                *input += 1;
        }

        if (neg) {
                return -r;
        }
        return r;
}

static void skip_newlines(const char **input) {
        while (**input == '\n') {
                *input += 1;
        }
}

static int parse_input(const char *input, long **numbers) {
        int len = 0;
        int cap = 8;
        *numbers = malloc(sizeof(**numbers)*cap);

        while (*input != '\0') {
                if (len >= cap) {
                        cap *= 2;
                        *numbers = realloc(*numbers, sizeof(**numbers)*cap);
                }
                int n = parse_int(&input);
                skip_newlines(&input);
                (*numbers)[len++] = n;
        }
        return len;
}

struct linked_list_node {
        long value;
        struct linked_list_node *next;
        struct linked_list_node *prev;
};

struct linked_list {
        struct linked_list_node *first;
        int len;
};

static void linked_list_init(struct linked_list *list, long *array, int len) {
        struct linked_list_node *nodes = malloc(sizeof(*nodes)*len);
        list->first = nodes;
        list->len = len;

#ifdef DEBUG
        fputs("Initial arrangement:\n", stderr);
#endif
        for (int i=0; i<len; i++) {
#ifdef DEBUG
                fprintf(stderr, "%ld, ", array[i]);
#endif
                nodes[i].value = array[i];
                nodes[i].next = &nodes[i+1];
                nodes[i].prev = &nodes[i-1];
        }
#ifdef DEBUG
        fputc('\n', stderr);
        fputc('\n', stderr);
#endif
        nodes[0].prev = &nodes[len-1];
        nodes[len-1].next = &nodes[0];
}

static void linked_list_free(struct linked_list *list) {
        free(list->first);
}

static void linked_list_to_array(const struct linked_list *list, long *array) {
        const struct linked_list_node *node = list->first;
        for (int i=0; i<list->len; i++) {
                array[i] = node->value;
                node = node->next;
        }
}

static struct linked_list_node *linked_list_get_original(struct linked_list *list, int idx) {
        return &list->first[idx];
}

static void linked_list_move_forward(struct linked_list_node *node, long amount) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        
        struct linked_list_node *n = node;
        while (amount > 0) {
                n = n->next;
                amount--;
        }

        n->next->prev = node;
        node->next = n->next;
        
        n->next = node;
        node->prev = n;
}

static void linked_list_move_backward(struct linked_list_node *node, long amount) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        
        struct linked_list_node *n = node;
        while (amount > 0) {
                n = n->prev;
                amount--;
        }

        n->prev->next = node;
        node->prev = n->prev;
        
        n->prev = node;
        node->next = n;
}

static long remove_wraparounds(long advance, int len) {
        return advance % (len-1);
}

static void mix(long *numbers, int len, int times) {
        struct linked_list list;
        linked_list_init(&list, numbers, len);
        for (int t=0; t<times; t++) {
                for (int i=0; i<len; i++) {
                        struct linked_list_node *node = linked_list_get_original(&list, i);
                        long n = remove_wraparounds(node->value, len);
                        if (n > 0) {
                                linked_list_move_forward(node, n);
                        } else if (n < 0) {
                                linked_list_move_backward(node, -n);
                        }
                }
                DBG("After %d rounds of mixing:", t+1);
                struct linked_list_node *node = list.first;
                for (int j=0; j<len; j++) {
#ifdef DEBUG
                        fprintf(stderr, "%ld, ", node->value);
#endif
                        node = node->next;
                }
#ifdef DEBUG
                fputc('\n', stderr);
                fputc('\n', stderr);
#endif
        }
        linked_list_to_array(&list, numbers);
        linked_list_free(&list);
}

static void solution(const char *const input, char *const output, int key, int times) {
        long *numbers;
        int len = parse_input(input, &numbers);
        for (int i=0; i<len; i++) {
                numbers[i] *= key;
        }

        mix(numbers, len, times);

        long res = 0;
        for (int i=0; i<len; i++) {
                if (numbers[i] == 0) {
                        DBG("%ld,%ld,%ld", numbers[(i+1000) % len], numbers[(i+2000) % len], numbers[(i+3000) % len]);
                        res = numbers[(i+1000) % len];
                        res += numbers[(i+2000) % len];
                        res += numbers[(i+3000) % len];
                        break;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
        free(numbers);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, 1, 1);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, 811589153, 10);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
