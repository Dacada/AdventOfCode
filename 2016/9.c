#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static size_t strlen_nospace(const char *const input) {
        size_t count = 0;
        for (size_t i=0;; i++) {
                if (input[i] == '\0') {
                        break;
                }
                if (!isspace(input[i])) {
                        count++;
                }
        }
        return count;
}

static size_t parse_number(const char **const input) {
        size_t n = 0;
        char c;
        
        while (isdigit(c=**input)) {
                n = n * 10 + c - '0';
                *input += 1;
        }
        
        return n;
}

static size_t parse_number_idx(const char *const input, size_t *const idx) {
        size_t n=0;
        char c;
        while (isdigit(c=input[*idx])) {
                n = n*10+c-'0';
                *idx += 1;
        }
        return n;
}

struct string_builder {
        char *string;
        size_t size;
        size_t capacity;
};

static void string_builder_init(struct string_builder *const str) {
        str->capacity = 8;
        str->string = malloc(sizeof(str->string)*str->capacity);
        str->size = 0;
}

static void string_builder_append_array(struct string_builder *const str,
                                        const char *const array, const size_t size) {
        bool grow = false;
        while (str->size + size >= str->capacity) {
                str->capacity *= 2;
                grow = true;
        }

        if (grow) {
                str->string = realloc(str->string, sizeof(str->string)*str->capacity);
        }

        strncpy(str->string + str->size, array, size);
        str->size += size;
}

static void string_builder_append_char(struct string_builder *const str, const char c) {
        if (str->size >= str->capacity) {
                str->capacity *= 2;
                str->string = realloc(str->string, sizeof(str->string)*str->capacity);
        }

        str->string[str->size] = c;
        str->size += 1;
}

static char *string_builder_finish(struct string_builder *const str) {
        string_builder_append_char(str, '\0');
        return str->string;
}

static char *decompress(const char *input, bool *markers) {
        *markers = false;
        
        struct string_builder result;
        string_builder_init(&result);

        for (;; input++) {
                if (*input == '\0') {
                        break;
                }

                if (*input == '(') {
                        *markers = true;
                        
                        input++;
                        size_t repeat_size = parse_number(&input);
                        ASSERT(*input == 'x', "parse error");
                        input++;
                        size_t repeat_times = parse_number(&input);
                        ASSERT(*input == ')', "parse error");
                        input++;

                        char repeat[repeat_size];
                        strncpy(repeat, input, repeat_size);

                        for (size_t i=0; i<repeat_times; i++) {
                                string_builder_append_array(&result, repeat, repeat_size);
                        }

                        input += repeat_size-1;
                } else {
                        string_builder_append_char(&result, *input);
                }
        }

        return string_builder_finish(&result);
}

static size_t decompressed_length(const char *const text, size_t size) {
        size_t count = 0;
        for (size_t i=0; i<size; i++) {
                if (text[i] == '(') {
                        i++;
                        size_t repeat_size = parse_number_idx(text, &i);
                        ASSERT(text[i] == 'x', "parse error");
                        i++;
                        size_t repeat_times = parse_number_idx(text, &i);
                        ASSERT(text[i] == ')', "parse error");
                        i++;

                        size_t fragment_size = decompressed_length(text+i, repeat_size);
                        count += fragment_size * repeat_times;
                        i += repeat_size-1;
                } else {
                        count++;
                }
        }
        return count;
}

static void solution1(const char *const input, char *const output) {
        bool aux;
        char *decompressed = decompress(input, &aux);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", strlen_nospace(decompressed));
        free(decompressed);
}

static void solution2(const char *const input, char *const output) {
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", decompressed_length(input, strlen_nospace(input)));
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
