#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

static int parse_number(const char **const input) {
        ASSERT(isdigit(**input), "parse error");

        int n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }
        return n;
}

struct dot {
        int x;
        int y;
};

static int dot_cmp(const void *a, const void *b) {
        const struct dot *dot1 = a;
        const struct dot *dot2 = b;

        if (dot1->x > dot2->x) {
                return 1;
        } else if (dot1->x < dot2->x) {
                return -1;
        } else {
                if (dot1->y > dot2->y) {
                        return 1;
                } else if (dot1->y < dot2->y) {
                        return -1;
                } else {
                        return 0;
                }
        }
}

static struct dot parse_dot(const char **const input) {
        struct dot dot;
        dot.x = parse_number(input);
        ASSERT(**input == ',', "parse error");
        *input += 1;
        dot.y = parse_number(input);
        ASSERT(**input == '\n', "parse error");
        *input += 1;
        return dot;
}

static struct dot *parse_input_dots(const char **const input, size_t *const len) {
        size_t size = 16;
        *len = 0;
        struct dot *list = malloc(size * sizeof(*list));

        while (**input != '\n') {
                if (*len >= size) {
                        size *= 2;
                        list = realloc(list, size * sizeof(*list));
                }
                list[*len] = parse_dot(input);
                *len += 1;
        }
        *input += 1;
        ASSERT(**input != '\n', "parse error");

        return list;
}

struct fold {
        enum {
                AXIS_X,
                AXIS_Y,
        } axis;
        int value;
};

static struct fold parse_fold(const char **const input) {
        struct fold fold;

        const char *const msg = "fold along ";
        ASSERT(strncmp(*input, msg, strlen(msg)) == 0, "parse error");
        *input += strlen(msg);

        switch (**input) {
        case 'x':
                fold.axis = AXIS_X;
                break;
        case 'y':
                fold.axis = AXIS_Y;
                break;
        default:
                FAIL("parse error");
        }
        *input += 1;
        ASSERT(**input == '=', "parse error");
        *input += 1;

        fold.value = parse_number(input);

        return fold;
}

static struct fold *parse_input_folds(const char **const input, size_t *const len) {
        size_t size = 16;
        *len = 0;
        struct fold *list = malloc(size * sizeof(*list));

        while (**input != '\0') {
                if (*len >= size) {
                        size *= 2;
                        list = realloc(list, size * sizeof(*list));
                }
                list[*len] = parse_fold(input);
                *len += 1;

                ASSERT(**input == '\n' || **input == '\0', "parse error");
                if (**input == '\n') {
                        *input += 1;
                }
        }

        return list;
}

#define REFLECT(dots, ndots, axis, value)                               \
        for (size_t i=0; i<(ndots); i++) {                              \
                if ((dots)[i].axis < (value)) {                         \
                        continue;                                       \
                } else if ((dots)[i].axis > (value)) {                  \
                        int diff = (dots)[i].axis - (value);            \
                        (dots)[i].axis = (value) - diff;                \
                } else {                                                \
                        FAIL("dot present in folding axis line");       \
                }                                                       \
        }

static void do_apply_fold(struct dot *const dots, const size_t ndots, const struct fold fold) {
        switch(fold.axis) {
        case AXIS_X:
                REFLECT(dots, ndots, x, fold.value)
                break;
        case AXIS_Y:
                REFLECT(dots, ndots, y, fold.value)
                break;
        default:
                FAIL("invalid enumeration value");
        }
}

static size_t remove_duplicates(struct dot *const list, const size_t len) {
        qsort(list, len, sizeof(*list), dot_cmp);

        size_t i, j=0;
        for (i=0; i<len; i++) {
                if (j >= len) {
                        break;
                }
                list[i] = list[j];
                do {
                        j++;
                } while (j<len && dot_cmp(list+i, list+j) == 0);
        }

        return i;
}

static size_t apply_fold(struct dot *const dots, const size_t ndots, const struct fold fold) {
        do_apply_fold(dots, ndots, fold);
        return remove_duplicates(dots, ndots);
}

static char *print_dots(const struct dot *const dots, const size_t ndots, size_t *buff_w, size_t *buff_h) {
        struct dot max = {.x=INT_MIN, .y=INT_MIN};
        struct dot min = {.x=INT_MAX, .y=INT_MAX};
        
        for (size_t i=0; i<ndots; i++) {
                struct dot dot = dots[i];
                if (dot.x > max.x) {
                        max.x = dot.x;
                }
                if (dot.y > max.y) {
                        max.y = dot.y;
                }
                if (dot.x < min.x) {
                        min.x = dot.x;
                }
                if (dot.y < min.y) {
                        min.y = dot.y;
                }
        }

	size_t buffer_cap = 50;
	char *buffer = malloc(sizeof(*buffer)*buffer_cap);
	size_t buffer_len = 0;
	size_t buffer_width = 0;
	size_t buffer_height = 0;

        for (int j=min.y; j<=max.y; j++) {
                for (int i=min.x; i<=max.x; i++) {
                        struct dot d = {.x=i, .y=j};

                        bool found = false;
                        for (size_t k=0; k<ndots; k++) {
                                if (dot_cmp(dots+k, &d) == 0) {
                                        found = true;
                                }
                        }
			
		  if (buffer_len >= buffer_cap) {
		    buffer_cap *= 2;
		    buffer = realloc(buffer, sizeof(*buffer)*buffer_cap);
		  }

                        if (found) {
			  buffer[buffer_len++] = '#';
                        } else {
			  buffer[buffer_len++] = ' ';
                        }
                }
		if (buffer_height == 0) {
		  buffer_width = buffer_len;
		}
		buffer_height++;
        }

	*buff_w = buffer_width;
	*buff_h = buffer_height;
	return buffer;
}

static void solution1(const char *input, char *const output) {
        size_t ndots, nfolds;
        struct dot *dots = parse_input_dots(&input, &ndots);
        struct fold *folds = parse_input_folds(&input, &nfolds);

        ndots = apply_fold(dots, ndots, folds[0]);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", ndots);
        free(dots);
        free(folds);
}

static void solution2(const char *input, char *const output) {
        size_t ndots, nfolds;
        struct dot *dots = parse_input_dots(&input, &ndots);
        struct fold *folds = parse_input_folds(&input, &nfolds);

        for (size_t i=0; i<nfolds; i++) {
                ndots = apply_fold(dots, ndots, folds[i]);
        }

	size_t buffer_width, buffer_height;
        char *buffer = print_dots(dots, ndots, &buffer_width, &buffer_height);
	#ifdef DEBUG
	for (size_t j=0; j<buffer_height; j++) {
	  for (size_t i=0; i<buffer_width; i++) {
	    fputc(buffer[j*buffer_width+i], stderr);
	  }
	  fputc('\n', stderr);
	}
	#endif
	char *result = aoc_ocr(buffer, buffer_width, buffer_height);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", result);
        free(dots);
        free(folds);
	free(buffer);
	free(result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
