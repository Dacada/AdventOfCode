#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static int spelled_out_digit(const char **input) {
	const char *digits[] = {
		"one",
		"two",
		"three",
		"four",
		"five",
		"six",
		"seven",
		"eight",
		"nine",
	};

	for (int i=0; i<9; i++) {
		const char *digit = digits[i];
		if (strncmp(*input, digit, strlen(digit)) == 0) {
			return i + 1;
		}
	}
	return -1;
}

static int parse_number_1(const char **input) {
	while (**input != '\n' && **input != '\0') {
		if (isdigit(**input)) {
			return **input - '0';
		}
		*input += 1;
	}
	return -1;
}

static int parse_number_2(const char **input) {
	while (**input != '\n' && **input != '\0') {
		if (isdigit(**input)) {
			return **input - '0';
		}

		int n = spelled_out_digit(input);
		if (n >= 0) {
			return n;
		}

		*input += 1;
	}
	return -1;
}

static void parse_line(const char **input, int *first, int *last,
		       int (*parse_number)(const char **)) {
	bool found_first = false;
	*first = *last = 0;
	for (;;) {
		int n = parse_number(input);
		if (n < 0) {
			break;
		}
		
		if (!found_first) {
			found_first = true;
			*first = n;
		}
		*last = n;

		*input += 1;
	}
	if (**input != '\0') {
		*input += 1;
	}
}

static int parse_input(const char *input, int **numbers,
		       int (*parse_number)(const char **)) {
	int size = 0;
	int capacity = 16;
	*numbers = malloc(sizeof(*numbers) * capacity);

	while (*input != '\0') {
		int first, last;
		parse_line(&input, &first, &last, parse_number);

		if (size >= capacity) {
			capacity *= 2;
			*numbers = realloc(*numbers, sizeof(*numbers) * capacity);
		}
		(*numbers)[size++] = first * 10 + last;
		DBG("%d %d %d", first, last, (*numbers)[size-1]);
	}

	return size;
}

static void solution(const char *const input, char *const output,
		     int (*parse_number)(const char **)) {
        int *numbers;
	int size = parse_input(input, &numbers, parse_number);
	int total = 0;
	for (int i=0; i<size; i++) {
		total += numbers[i];
	}
	free(numbers);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", total);
}

static void solution1(const char *const input, char *const output) {
	solution(input, output, parse_number_1);
}

static void solution2(const char *const input, char *const output) {
	solution(input, output, parse_number_2);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
