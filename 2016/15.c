#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NDISKS 6

struct disk {
        int positions;
        int initial;
};

static bool is_disk_at_position_on_time(const struct disk *const disk,
                                        int position, int time) {
        while (position < 0) {
                position += disk->positions;
        }
        position %= disk->positions;

        int position_at_time = (disk->initial + time) % disk->positions;

        return position == position_at_time;
}

static void parse_skip(const char **const input, const char *const expect) {
        size_t len = strlen(expect);
        ASSERT(strncmp(*input, expect, len) == 0, "parse error");
        *input += len;
}

static int parse_number(const char **const input) {
        int n = 0;
        while (isdigit(**input)) {
                n *= 10;
                n += **input - '0';
                *input += 1;
        }
        return n;
}

static void parse_disk(const char **const input, struct disk *const disk) {
        parse_skip(input, "Disc #");
        ASSERT(isdigit(**input), "parse error");
        *input += 1;
        parse_skip(input, " has ");
        disk->positions = parse_number(input);
        parse_skip(input, " positions; at time=0, it is at position ");
        disk->initial = parse_number(input);
        parse_skip(input, ".\n");
}

static void parse(const char *input, struct disk *disks) {
        while (*input) {
                parse_disk(&input, disks);
                if (*input == '\n') {
                        break;
                }
                disks++;
        }
}

static void solution(const char *const input, char *const output, bool extradisk) {
        struct disk disks[NDISKS+1];
        parse(input, disks);

        int ndisks = NDISKS;
        if (extradisk) {
                disks[ndisks].initial = 0;
                disks[ndisks].positions = 11;
                ndisks++;
        }

        int t;
        for (t=0;; t++) {
                bool all = true;

                for (int i=0; i<ndisks; i++) {
                        all &= is_disk_at_position_on_time(disks+i, -i-1, t);
                        if (!all) {
                                break;
                        }
                }
                
                if (all) {
                        break;
                }
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", t);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, false);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, true);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
