#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>

struct sequence {
	int cap;
	int len;
	int *list;
};

static int predict(const struct sequence *seq, int direction) {
	bool all_zeros = true;
	for (int i=0; i<seq->len; i++) {
		if (seq->list[i] != 0) {
			all_zeros = false;
			break;
		}
	}
	if (all_zeros) {
		return 0;
	}

	struct sequence newseq;
	newseq.cap = seq->len - 1;
	newseq.len = seq->len - 1;
	newseq.list = malloc(sizeof(*newseq.list)*newseq.cap);

	for (int i=0; i<seq->len-1; i++) {
		newseq.list[i] = seq->list[i+1] - seq->list[i];
	}
	int n = predict(&newseq, direction);

	free(newseq.list);
	if (direction == 0) {
		return seq->list[seq->len-1] + n;
	} else {
		return seq->list[0] - n;
	}
}

static void skip_blank(const char **input) {
	while (isblank(**input)) {
		*input += 1;
	}
}

static int parse_int(const char **input) {
	bool neg = **input == '-';
	if (neg) {
		*input += 1;
	}
		
	ASSERT(isdigit(**input), "parse error '%d' '%c'", **input, **input);

	int n = 0;
	while (isdigit(**input)) {
		n *= 10;
		n += **input - '0';
		*input += 1;
	}

	if (neg) {
		n = -n;
	}
	
	return n;
}

static void parse_line(const char **input, struct sequence *seq) {
	int len = 0;
	int cap = 4;
	int *list = malloc(sizeof(*list)*cap);

	while (**input != '\0' && **input != '\n') {
		if (len >= cap) {
			cap *= 2;
			list = realloc(list, sizeof(*list)*cap);
		}
		int n = parse_int(input);
		list[len++] = n;
		skip_blank(input);
	}

	if (**input == '\n') {
		*input += 1;
	}

	seq->len = len;
	seq->cap = cap;
	seq->list = list;
}

static struct sequence *parse_input(const char *input, int *nseqs) {
	int len = 0;
	int cap = 2;
	struct sequence *list = malloc(sizeof(*list)*cap);

	while (*input != '\0') {
		if (len >= cap) {
			cap *= 2;
			list = realloc(list, sizeof(*list)*cap);
		}

		parse_line(&input, &list[len++]);
	}

	*nseqs = len;
	return list;
}

static void solution(const char *const input, char *const output, int direction) {
	int nseqs;
	struct sequence *sequences = parse_input(input, &nseqs);

	int res = 0;
	for (int i=0; i<nseqs; i++) {
		int n = predict(sequences+i, direction);
		res += n;
	}

	for (int i=0; i<nseqs; i++) {
		free(sequences[i].list);
	}
	free(sequences);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

static void solution1(const char *const input, char *const output) {
	solution(input, output, 0);
}

static void solution2(const char *const input, char *const output) {
	solution(input, output, 1);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
