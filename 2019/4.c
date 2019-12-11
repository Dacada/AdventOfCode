#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static bool smaller(char n1[6], char n2[6]) {
        for (int i=0; i<6; i++) {
                if (n1[i] > n2[i]) {
                        return false;
                } else if (n1[i] < n2[i]) {
                        return true;
                }
        }
        return true;
}

static void advance(char n[6], int i) {
        if (n[i] == '9') {
                n[i] = '0';
                if (i > 0) {
                        advance(n, i-1);
                }
        } else {
                n[i]++;
        }
}

static bool is_valid1(char n[6]) {
        char c = n[0];
        bool found_dub = false;
        for (int i=1; i<6; i++) {
                if (c > n[i]) {
                        return false;
                }
                if (c == n[i]) {
                        found_dub = true;
                }
                c = n[i];
        }
        return found_dub;
}

static bool is_valid2(char n[6]) {
        char c = n[0];
        
        char current_dub_track = c;
        int current_dub_count = 1;
        bool found_single_dub = false;
        
        for (int i=1; i<6; i++) {
                if (c > n[i]) {
                        return false;
                }
                
                if (n[i] == current_dub_track) {
                        current_dub_count++;
                } else {
                        if (current_dub_count == 2) {
                                found_single_dub = true;
                        }
                        current_dub_track = n[i];
                        current_dub_count = 1;
                }
                
                c = n[i];
        }
        return found_single_dub || current_dub_count == 2;
}

static void solution(const char *const input, char *const output, bool(*is_valid)(char[6])) {
        char num[6], limit[6];
        
        memcpy(num, input, 6);
        memcpy(limit, input+7, 6);

        int count = 0;
        while (smaller(num, limit)) {
                advance(num, 5);
                if (is_valid(num)) {
                        count++;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, is_valid1);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, is_valid2);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
