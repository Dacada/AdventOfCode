#include <aoclib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define LAST_ONE_MASK (1UL)
#define LAST_ZERO_MASK (~1UL)
#define MASKSIZE 36

struct memory_tree {
        bool empty;
        uint64_t address;
        uint64_t value;
        struct memory_tree *left;
        struct memory_tree *right;
};

static void memory_tree_init(struct memory_tree *const tree) {
        tree->empty = true;
        tree->left = tree->right = NULL;
}

static unsigned long memory_tree_get(const struct memory_tree *const tree, uint64_t addr) {
        if (tree->empty) {
                return 0;
        }

        if (addr == tree->address) {
                return tree->value;
        } else if (addr > tree->address) {
                return memory_tree_get(tree->left, addr);
        } else {
                return memory_tree_get(tree->right, addr);
        }
}

static void memory_tree_set(struct memory_tree *const tree, uint64_t addr, uint64_t value) {
        if (tree->empty) {
                tree->empty = false;
                tree->address = addr;
                tree->value = value;
                tree->left = malloc(sizeof(struct memory_tree));
                tree->right = malloc(sizeof(struct memory_tree));
                memory_tree_init(tree->left);
                memory_tree_init(tree->right);
                return;
        }

        if (addr == tree->address) {
                tree->value = value;
        } else if (addr > tree->address) {
                memory_tree_set(tree->left, addr, value);
        } else {
                memory_tree_set(tree->right, addr, value);
        }
}

static void memory_tree_free(struct memory_tree *const tree) {
        if (tree->empty) {
                return;
        }
        memory_tree_free(tree->left);
        memory_tree_free(tree->right);
        free(tree->left);
        free(tree->right);
}

static uint64_t parse_int(const char *const input, uint64_t *const i) {
        uint64_t n = 0;
        char c;
        while (isdigit(c=input[*i])) {
                n = n * 10 + c - '0';
                *i += 1;
        }
        return n;
}

static bool parse_input(const char *const input, size_t *const i, uint64_t *const a, uint64_t *const b) {
        bool res;
        if (strncmp(input+*i, "mask", 4) == 0) {
                res = true;
                *i += 4;
                ASSERT(strncmp(input+*i, " = ", 3) == 0, "parse error");
                *i += 3;
                
                *a = 0;
                *b = 0;
                for (int j=0; j<MASKSIZE; j++) {
                        *a <<= 1;
                        *b <<= 1;
                        switch (input[*i+j]) {
                        case 'X':
                                *a |= LAST_ONE_MASK;
                                *b &= LAST_ZERO_MASK;
                                break;
                        case '0':
                                *a &= LAST_ZERO_MASK;
                                *b &= LAST_ZERO_MASK;
                                break;
                        case '1':
                                *a |= LAST_ONE_MASK;
                                *b |= LAST_ONE_MASK;
                                break;
                        default:
                                FAIL("parse error");
                        }
                }
                *i += MASKSIZE;
        } else if (strncmp(input+*i, "mem", 3) == 0) {
                res = false;
                *i += 3;
                ASSERT(input[*i] == '[', "parse error");
                *i += 1;
                *a = parse_int(input, i);
                ASSERT(input[*i] == ']', "parse error");
                *i += 1;
                ASSERT(strncmp(input+*i, " = ", 3) == 0, "parse error");
                *i += 3;
                *b = parse_int(input, i);
        } else {
                FAIL("parse error");
        }
        return res;
}

static void memory_set(struct memory_tree *const memory, unsigned long *total,
                       uint64_t address, uint64_t value) {
        *total -= memory_tree_get(memory, address);
        memory_tree_set(memory, address, value);
        *total += value;
}

static void solution1(const char *const input, char *const output) {
        unsigned long total = 0;
        struct memory_tree memory;
        memory_tree_init(&memory);
        uint64_t and_mask = 0;
        uint64_t or_mask = 0;
        
        for (size_t i=0; input[i]!=0; i++) {
                uint64_t a, b;
                if (parse_input(input, &i, &a, &b)) {
                        and_mask = a;
                        or_mask = b;
                } else {
                        memory_set(&memory, &total, a, (b & and_mask) | or_mask);
                }
                ASSERT(input[i] == '\n', "parse error");
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", total);
        memory_tree_free(&memory);
}

static bool parse_input_2(const char *const input, size_t *const i, char mask[MASKSIZE+1],
                          uint64_t *const address, uint64_t *const value) {
        if (strncmp(input+*i, "mask", 4) == 0) {
                *i += 4;
                ASSERT(strncmp(input+*i, " = ", 3) == 0, "parse error");
                *i += 3;
                strncpy(mask, input+*i, MASKSIZE);
                *i += MASKSIZE;
                return false;
        } else if (strncmp(input+*i, "mem", 3) == 0) {
                *i += 3;
                ASSERT(input[*i] == '[', "parse error");
                *i += 1;
                *address = parse_int(input, i);
                ASSERT(input[*i] == ']', "parse error");
                *i += 1;
                ASSERT(strncmp(input+*i, " = ", 3) == 0, "parse error");
                *i += 3;
                *value = parse_int(input, i);
                return true;
        } else {
                FAIL("parse error");
        }
}

static void apply_mask(uint64_t address, const char *mask, uint64_t *const addresses,
                           unsigned *const addresses_size) {
        *addresses_size = 1;
        addresses[0] = 0;
        for (unsigned i=0; i<MASKSIZE; i++) {
                switch (mask[i]) {
                case 'X':
                        for (unsigned j=0; j<*addresses_size; j++) {
                                addresses[j] <<= 1;
                                addresses[j] &= LAST_ZERO_MASK;
                        }
                        for (unsigned j=*addresses_size; j<(*addresses_size)*2; j++) {
                                addresses[j] = addresses[j - *addresses_size];
                                addresses[j] |= LAST_ONE_MASK;
                        }
                        *addresses_size *= 2;
                        break;
                case '0':
                        for (unsigned j=0; j<*addresses_size; j++) {
                                addresses[j] <<= 1;
                                if (address & (1UL<<(35-i))) {
                                        addresses[j] |= LAST_ONE_MASK;
                                } else {
                                        addresses[j] &= LAST_ZERO_MASK;
                                }
                        }
                        break;
                case '1':
                        for (unsigned j=0; j<*addresses_size; j++) {
                                addresses[j] <<= 1;
                                addresses[j] |= LAST_ONE_MASK;
                        }
                        break;
                default:
                        FAIL("unexpected mask");
                }
        }
}

static void memory_set_2(struct memory_tree *const memory, const char mask[MASKSIZE],
                         const uint64_t address, const uint64_t value, unsigned long *const total) {
        uint64_t addresses[1024];
        unsigned n;
        apply_mask(address, mask, addresses, &n);
        for (unsigned i=0; i<n; i++) {
                uint64_t addr = addresses[i];
                *total -= memory_tree_get(memory, addr);
                memory_tree_set(memory, addr, value);
        }
        *total += value*n;
}

__attribute__((pure))
static unsigned long memory_tree_sum(const struct memory_tree *const tree) {
        if (tree->empty) {
                return 0;
        }
        return tree->value + memory_tree_sum(tree->left) + memory_tree_sum(tree->right);
}

static void solution2(const char *const input, char *const output) {
        unsigned long total = 0;
        struct memory_tree memory;
        memory_tree_init(&memory);
        char mask[MASKSIZE+1];

        for (size_t i=0; input[i]!=0; i++) {
                uint64_t addr, val;
                if (parse_input_2(input, &i, mask, &addr, &val)) {
                        memory_set_2(&memory, mask, addr, val, &total);
                }
        }

        total = memory_tree_sum(&memory);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", total);
        memory_tree_free(&memory);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
