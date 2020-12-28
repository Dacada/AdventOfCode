#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <stdio.h>
#include <string.h>

static bool istls(const char *const ip) {
        bool inbrackets = false;
        bool found_abba = false;

        char prevchars[3] = { '\0', '\0', '\0' };
        
        for (unsigned i=0;; i++) {
                char c = ip[i];
                if (c == '\0') {
                        return found_abba;
                }

                if (c == '[') {
                        inbrackets = true;
                        prevchars[0] = '\0';
                        prevchars[1] = '\0';
                        prevchars[2] = '\0';
                        continue;
                } else if (c == ']') {
                        inbrackets = false;
                        prevchars[0] = '\0';
                        prevchars[1] = '\0';
                        prevchars[2] = '\0';
                        continue;
                }

                if (!found_abba || inbrackets) {
                        if (prevchars[1] == prevchars[2] &&
                            prevchars[0] == c &&
                            prevchars[0] != prevchars[1]) {
                                if (inbrackets) {
                                        return false;
                                } else {
                                        found_abba = true;
                                }
                        } else {
                                prevchars[0] = prevchars[1];
                                prevchars[1] = prevchars[2];
                                prevchars[2] = c;
                        }
                }
        }
}

static bool isssl(const char *const ip) {
        char abas[256][3];
        char babs[256][3];

        int nabas = 0;
        int nbabs = 0;

        char curr[2] = { '\0', '\0' };
        
        bool inbrackets = false;
        for (unsigned i=0;; i++) {
                char c = ip[i];
                if (c == '\0') {
                        break;
                }

                if (c == '[') {
                        inbrackets = true;
                        curr[0] = '\0';
                        curr[1] = '\0';
                        continue;
                } else if (c == ']') {
                        inbrackets = false;
                        curr[0] = '\0';
                        curr[1] = '\0';
                        continue;
                }

                if (curr[0] == c && curr[1] != c) {
                        if (inbrackets) {
                                babs[nbabs][0] = curr[0];
                                babs[nbabs][1] = curr[1];
                                babs[nbabs][2] = c;
                                nbabs++;
                        } else {
                                abas[nabas][0] = curr[0];
                                abas[nabas][1] = curr[1];
                                abas[nabas][2] = c;
                                nabas++;
                        }
                }
                curr[0] = curr[1];
                curr[1] = c;
        }

        for (int j=0; j<nbabs; j++) {
                for (int i=0; i<nabas; i++) {
                        if (abas[i][0] == babs[j][1] &&
                            abas[i][1] == babs[j][0]) {
                                return true;
                        }
                }
        }
        
        return false;
}

static char **parse(const char *input) {
        size_t capacity = 16;
        char **list = malloc(capacity * sizeof *list);
        size_t length = 0;

        for (;;) {
                const char *eol = strchr(input, '\n');
                if (eol == NULL) {
                        break;
                }
                
                char *ip = strndup(input, eol-input);
                list[length++] = ip;
                
                if (length >= capacity) {
                        capacity *= 2;
                        list = realloc(list, capacity * sizeof *list);
                }

                input = eol+1;
                if (*input == '\0') {
                        break;
                }
        }

        list[length] = NULL;
        return list;
}

static void solution(const char *const input, char *const output, bool(*isgoal)(const char *)) {
        char **ips = parse(input);

        unsigned count = 0;
        for (unsigned i=0;; i++) {
                char *ip = ips[i];
                if (ip == NULL) {
                        break;
                }
                if (isgoal(ip)) {
                        DBG("%s yes", ip);
                        count++;
                } else {
                        DBG("%s no", ip);
                }
                free(ip);
        }
        free(ips);
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", count);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, istls);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, isssl);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
