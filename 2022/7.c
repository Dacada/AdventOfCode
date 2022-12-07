#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK_CAPACITY 1024

struct entry {
        char *name;
        struct entry *parent;
        size_t size;
        struct entry **children;
        int num_children;
        int cap_children;
};

static int cmp_entry(const void *e1, const void *e2) {
        const struct entry *const *entry1 = e1;
        const struct entry *const *entry2 = e2;
        return strcmp((*entry1)->name, (*entry2)->name);
}

static void add_entry_child(struct entry *const parent, struct entry *const child) {
        parent->num_children += 1;
        if (parent->num_children > parent->cap_children) {
                parent->cap_children *= 2;
                parent->children = realloc(parent->children, parent->cap_children * sizeof(*parent->children));
        }
        parent->children[parent->num_children - 1] = child;
        qsort(parent->children, parent->num_children, sizeof(*parent->children), cmp_entry);
}

static struct entry *find_child_entry(const struct entry *const entry,  char *const name) {
        static struct entry key;
        static struct entry *key_ptr = &key;
        key.name = name;
        
        struct entry **res = bsearch(&key_ptr, entry->children, entry->num_children, sizeof(*entry->children), cmp_entry);
        if (res == NULL) {
                return NULL;
        } else {
                return *res;
        }
}

static struct entry *create_entry(char *const name, struct entry *parent) {
        struct entry *entry = malloc(sizeof(*entry));
        entry->name = strdup(name);
        entry->parent = parent;
        if (parent != NULL) {
                add_entry_child(parent, entry);
        }
        return entry;
}

static struct entry *create_dir_entry(char *const name, struct entry *parent) {
        struct entry *entry = create_entry(name, parent);
        entry->size = 0;
        entry->num_children = 0;
        entry->cap_children = 8;
        entry->children = malloc(sizeof(*entry->children) * entry->cap_children);
        return entry;
}

static struct entry *create_file_entry(char *const name, size_t size, struct entry *parent) {
        struct entry *entry = create_entry(name, parent);
        entry->size = size;
        entry->num_children = entry->cap_children = 0;
        entry->children = NULL;
        return entry;
}

static struct entry *find_or_create_child_dir(struct entry *const entry, char *const name) {
        struct entry *new = find_child_entry(entry, name);
        if (new == NULL) {
                new = create_dir_entry(name, entry);
        }
        return new;
}

static struct entry *find_or_create_child_file(struct entry *const entry, char *const name, size_t size) {
        struct entry *new = find_child_entry(entry, name);
        if (new == NULL) {
                new = create_file_entry(name, size, entry);
        } else {
                ASSERT(new->size == size, "logic error");
        }
        return new;
}

static void free_entry(struct entry *entry) {
        free(entry->name);
        for (int i=0; i<entry->num_children; i++) {
                free_entry(entry->children[i]);
        }
        free(entry->children);
        free(entry);
}

static char *extract_until_after_eol(const char **const input) {
        int eol = 0;
        while ((*input)[eol] != '\n' && (*input)[eol] != '\0') {
                eol++;
        }
        char *str = strndup(*input, eol);
        *input += eol;
        while (**input == '\n') {
                *input += 1;
        }
        return str;
}

#define ASSERT_STR(input, str)                                          \
        ASSERT(strncmp(input, str, sizeof(str)-1) == 0, "parse error"); \
        input += sizeof(str)-1;

static struct entry *parse_cd(const char **const input, struct entry *entry) {
        ASSERT_STR(*input, "$ cd ");
        if (**input == '.') {
                ASSERT_STR(*input, "..\n");
                return entry->parent;
        }

        char *name = extract_until_after_eol(input);
        DBG("$ cd %s", name);
        struct entry *res = find_or_create_child_dir(entry, name);
        free(name);
        return res;
}

static void parse_ls_dir(const char **const input, struct entry *const entry) {
        ASSERT_STR(*input, "dir ");
        char *name = extract_until_after_eol(input);
        DBG("dir %s", name);
        find_or_create_child_dir(entry, name);
        free(name);
}

static size_t parse_int(const char **const input) {
        size_t res = 0;
        ASSERT(isdigit(**input), "parse error");
        while (isdigit(**input)) {
                res *= 10;
                res += **input - '0';
                *input += 1;
        }
        return res;
}

static void parse_ls_file(const char **const input, struct entry *const entry) {
        size_t size = parse_int(input);
        ASSERT_STR(*input, " ");
        char *name = extract_until_after_eol(input);
        DBG("%lu %s", size, name);
        find_or_create_child_file(entry, name, size);
        free(name);
}

static void parse_ls(const char **const input, struct entry *const entry) {
        ASSERT_STR(*input, "$ ls\n");
        DBG("$ ls");
        while (**input != '$' && **input != '\0') {
                if (**input == 'd') {
                        parse_ls_dir(input, entry);
                } else if (isdigit(**input)) {
                        parse_ls_file(input, entry);
                } else {
                        FAIL("parse error");
                }
        }
}

static struct entry *parse_input(const char *input) {
        ASSERT_STR(input, "$ cd /\n");
        DBG("$ cd /");
        struct entry *root = create_dir_entry("/", NULL);
        
        struct entry *entry = root;
        while (*input != '\0') {
                if (input[2] == 'c') {
                        entry = parse_cd(&input, entry);
                } else if (input[2] == 'l') {
                        parse_ls(&input, entry);
                } else {
                        FAIL("parse error");
                }
        }

        return root;
}

#undef ASSERT_STR

// return total size of entry, keep track of smaller entries in arguments
static size_t find_directory_sizes(struct entry *const entry,
                                   size_t **dirs, int *ndirs, int *cdirs) {
        if (entry->size > 0) {  // a file
                return entry->size;
        }
        
        size_t result = 0;
        for (int i=0; i<entry->num_children; i++) {
                result += find_directory_sizes(entry->children[i], dirs, ndirs, cdirs);
        }

        *ndirs += 1;
        if (*ndirs > *cdirs) {
                *cdirs *= 2;
                *dirs = realloc(*dirs, sizeof(**dirs) * *cdirs);
        }
        (*dirs)[*ndirs - 1] = result;

        return result;
}

static size_t small_dirs(const char *const input, int *ndirs, size_t **dirs) {
        struct entry *root = parse_input(input);
        *ndirs = 0;
        int cdirs = 16;
        *dirs = malloc(sizeof(*dirs) * cdirs);
        size_t total_size = find_directory_sizes(root, dirs, ndirs, &cdirs);
        free_entry(root);
        return total_size;
}

static void solution1(const char *const input, char *const output) {
        int ndirs;
        size_t *dirs;
        small_dirs(input, &ndirs, &dirs);
        
        size_t total = 0;
        for (int i=0; i<ndirs; i++) {
                if (dirs[i] < 100000) {
                        total += dirs[i];
                }
        }
        free(dirs);

        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", total);
}

static void solution2(const char *const input, char *const output) {
        int ndirs;
        size_t *dirs;
        size_t current_size = small_dirs(input, &ndirs, &dirs);
        size_t unused_size = 70000000 - current_size;
        size_t required_size = 30000000 - unused_size;
        
        size_t result = 70000000;
        for (int i=0; i<ndirs; i++) {
                if (dirs[i] >= required_size && dirs[i] < result) {
                        result = dirs[i];
                }
        }
        free(dirs);

        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
