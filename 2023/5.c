#include <aoclib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

#define NMAPS 7
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

struct range {
	long start;
	long end;
};

static long range_contains(const struct range *r, long n) {
	if (n < r->start) {
		return -1;
	}
	if (n >= r->end) {
		return -2;
	}
	return n - r->start;
}

static long range_index(const struct range *r, long idx) {
	return r->start + idx;
}

static bool range_overlap(const struct range *r1, const struct range *r2,
			  struct range *left, struct range *right, struct range *overlap) {
	if (range_contains(r1, r2->start) < 0 && range_contains(r2, r1->start )< 0) {
		return false;
	}
	overlap->start = MAX(r1->start, r2->start);
	overlap->end = MIN(r1->end, r2->end);

	if (r1->start < r2->start) {
		left->start = r1->start;
		left->end = r2->start;
	} else {
		left->start = -1;
		left->end = -1;
	}

	if (r1->end > r2->end) {
		right->start = r2->end;
		right->end = r1->end;
	} else {
		right->start = -1;
		right->end = -1;
	}

	return true;
}

struct map {
	int nranges;
	struct range *input_ranges;
	struct range *output_ranges;
};

static void assert_str(const char **input, const char *exp) {
	int n = strlen(exp);
	ASSERT(strncmp(*input, exp, n) == 0, "parse error '%s' '%s'", *input, exp);
	*input += n;
}


static void skip_blank(const char **input) {
	while (isblank(**input)) {
		*input += 1;
	}
}

static void skip_until(const char **input, char until) {
	while (**input != until) {
		*input += 1;
	}
}

static long parse_long(const char **input) {
	ASSERT(isdigit(**input), "parse error '%d' '%c'", **input, **input);

	long n = 0;
	while (isdigit(**input)) {
		n *= 10;
		n += **input - '0';
		*input += 1;
	}
	return n;
}

static long *parse_seeds(const char **input, int *nseeds) {
	int len = 0;
	int cap = 2;
	long *res = malloc(sizeof(*res)*cap);

	assert_str(input, "seeds: ");
	skip_blank(input);
	while (isdigit(**input)) {
		if (len >= cap) {
			cap *= 2;
			res = realloc(res, sizeof(*res)*cap);
		}
		res[len++] = parse_long(input);
		skip_blank(input);
	}
	assert_str(input, "\n\n");

	*nseeds = len;
	return res;
}

static void parse_ranges(const char **input, struct range *input_range, struct range *output_range) {
	long output_start = parse_long(input);
	ASSERT(**input == ' ', "parse error");
	*input += 1;
	
	long input_start = parse_long(input);
	ASSERT(**input == ' ', "parse error");
	*input += 1;

	long length = parse_long(input);

	input_range->start = input_start;
	input_range->end = input_start + length;
	output_range->start = output_start;
	output_range->end = output_start + length;
}

static void parse_map(const char **input, struct map *map) {
	skip_until(input, ':');
	*input += 2;

	int cap = 2;
	int len = 0;
	struct range *input_ranges = malloc(sizeof(*input_ranges)*cap);
	struct range *output_ranges = malloc(sizeof(*output_ranges)*cap);

	while (**input != '\n' && **input != '\0') {
		if (len >= cap) {
			cap *= 2;
			input_ranges = realloc(input_ranges, sizeof(*input_ranges)*cap);
			output_ranges = realloc(output_ranges, sizeof(*output_ranges)*cap);
		}
		parse_ranges(input, &input_ranges[len], &output_ranges[len]);
		ASSERT(**input == '\n', "parse error");
		*input += 1;
		len++;
	}
	
	map->nranges = len;
	map->input_ranges = input_ranges;
	map->output_ranges = output_ranges;

	if (**input != '\0') {
		*input += 1;
	}
}

static void parse_maps(const char **input, struct map maps[NMAPS]) {
	for (int i=0; i<NMAPS; i++) {
		parse_map(input, &maps[i]);
	}
}

static void solution1(const char *input, char *const output) {
	int nseeds;
	long *seeds = parse_seeds(&input, &nseeds);
	
	struct map maps[NMAPS];
	parse_maps(&input, maps);

	long res = LONG_MAX;
	for (int i=0; i<nseeds; i++) {
		long element = seeds[i];
		for (int j=0; j<NMAPS; j++) {
			struct map *map = maps + j;
			for (int k=0; k<map->nranges; k++) {
				long idx = range_contains(&map->input_ranges[k], element);
				if (idx >= 0) {
					element = range_index(&map->output_ranges[k], idx);
					break;
				}
			}
		}
		if (element < res) {
			res = element;
		}
	}

	for (int i=0; i<NMAPS; i++) {
		free(maps[i].input_ranges);
		free(maps[i].output_ranges);
	}
	free(seeds);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
}

struct range_collection {
	int len;
	int cap;
	struct range *ranges;
};

static void range_collection_init(struct range_collection *c) {
	c->len = 0;
	c->cap = 4;
	c->ranges = malloc(sizeof(*c->ranges)*c->cap);
}

static void range_collection_add(struct range_collection *c, struct range element) {
	if (c->len >= c->cap) {
		c->cap *= 2;
		c->ranges = realloc(c->ranges, sizeof(*c->ranges)*c->cap);
	}
	c->ranges[c->len++] = element;
}

static void range_collection_extend(struct range_collection *dst, const struct range_collection *src) {
	int newlen = dst->len + src->len;

	bool cap_changed = false;
	while (newlen >= dst->cap) {
		dst->cap *= 2;
		cap_changed = true;
	}
	if (cap_changed) {
		dst->ranges = realloc(dst->ranges, sizeof(*dst->ranges)*dst->cap);
	}

	memcpy(dst->ranges + dst->len, src->ranges, src->len*sizeof(*src->ranges));
	dst->len = newlen;
}

static long range_collection_start(const struct range_collection *c) {
	long res = LONG_MAX;
	for (int i=0; i<c->len; i++) {
		long n = c->ranges[i].start;
		if (n < res) {
			res = n;
		}
	}
	return res;
}

static void range_collection_free(struct range_collection *c) {
	free(c->ranges);
}

static void range_collection_subdivide(struct range_collection *current,
				       struct range input, struct range output,
				       struct range_collection *successful_maps,
				       struct range_collection *unsuccessful_maps) {
	for (int i=0; i<current->len; i++) {
		struct range range = current->ranges[i];

		struct range left_overhang, right_overhang, overlap;
		if (range_overlap(&range, &input, &left_overhang, &right_overhang, &overlap)) {
			if (left_overhang.start >= 0) {
				range_collection_add(unsuccessful_maps, left_overhang);
			}
			if (right_overhang.start >= 0) {
				range_collection_add(unsuccessful_maps, right_overhang);
			}
			struct range output_overlap;
			output_overlap.start = output.start + overlap.start - input.start;
			output_overlap.end = output.end + overlap.end - input.end;
			range_collection_add(successful_maps, output_overlap);
		} else {
			range_collection_add(unsuccessful_maps, range);
		}
	}
}

static void parse_seed_ranges(const char **input, struct range_collection *ranges) {
	range_collection_init(ranges);

	assert_str(input, "seeds: ");
	skip_blank(input);
	while (isdigit(**input)) {
		long start = parse_long(input);
		skip_blank(input);
		long length = parse_long(input);
		skip_blank(input);
		
		struct range r;
		r.start = start;
		r.end = start + length;
		range_collection_add(ranges, r);
	}
}

static void solution2(const char *input, char *const output) {
	struct range_collection element_ranges;
	parse_seed_ranges(&input, &element_ranges);
	
	struct map maps[NMAPS];
	parse_maps(&input, maps);

	for (int j=0; j<NMAPS; j++) {
		struct map *map = maps + j;

		struct range_collection next_element_ranges;
		range_collection_init(&next_element_ranges);
		for (int k=0; k<map->nranges; k++) {
			struct range_collection unsuccessfully_mapped_ranges;
			range_collection_init(&unsuccessfully_mapped_ranges);
			
			range_collection_subdivide(&element_ranges, map->input_ranges[k], map->output_ranges[k],
						   &next_element_ranges, &unsuccessfully_mapped_ranges);

			range_collection_free(&element_ranges);
			element_ranges = unsuccessfully_mapped_ranges;
		}
		range_collection_extend(&next_element_ranges, &element_ranges);
		range_collection_free(&element_ranges);
		element_ranges = next_element_ranges;
	}

	long res = range_collection_start(&element_ranges);

	range_collection_free(&element_ranges);
	for (int i=0; i<NMAPS; i++) {
		free(maps[i].input_ranges);
		free(maps[i].output_ranges);
	}
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
