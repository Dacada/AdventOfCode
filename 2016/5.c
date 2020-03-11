#include <aoclib.h>
#include <bsd/md5.h>
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#endif

static void solution1(const char *const input, char *const output) {
        size_t len = strlen(input);
        MD5_CTX base_ctx, iter_ctx;

        MD5Init(&base_ctx);
        MD5Update(&base_ctx, (const uint8_t *const)input, len);

        char password[9]={0};
        int k = 0;
        
        for (unsigned i=0; k<8; i++) {
                memcpy(&iter_ctx, &base_ctx, sizeof(MD5_CTX));

                char istr[64];
                len = snprintf(istr, 64, "%u", i);
                MD5Update(&iter_ctx, (uint8_t*)istr, len);

                uint8_t digest[MD5_DIGEST_LENGTH];
                MD5Final(digest, &iter_ctx);

                if (digest[0] == 0 && digest[1] == 0 && digest[2] <= 15) {
                        if (digest[2] < 10) {
                                password[k] = '0' + digest[2];
                        } else {
                                password[k] = 'a' + digest[2] - 10;
                        }
                        k++;
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", password);
}

#ifdef DEBUG
static void endwin_wrapper(void) {
        endwin();
}
#endif

static void solution2(const char *const input, char *const output) {
        size_t len = strlen(input);
        MD5_CTX base_ctx, iter_ctx;

        MD5Init(&base_ctx);
        MD5Update(&base_ctx, (const uint8_t *const)input, len);

        char password[9]={0};
        int k = 0;

        #ifdef DEBUG
        initscr();
        atexit(endwin_wrapper);
        noecho();
        raw();
        nodelay(stdscr, true);
        keypad(stdscr, true);
        curs_set(0);
        
        srand(time(NULL));
        unsigned m = 0;
        #endif
        
        for (unsigned i=0; k<8; i++) {
                memcpy(&iter_ctx, &base_ctx, sizeof(MD5_CTX));

                char istr[64];
                len = snprintf(istr, 64, "%u", i);
                MD5Update(&iter_ctx, (uint8_t*)istr, len);

                uint8_t digest[MD5_DIGEST_LENGTH];
                MD5Final(digest, &iter_ctx);

                if (digest[0] == 0 && digest[1] == 0 && digest[2] <= 15) {
                        uint8_t position = digest[2];
                        if (position < 8 && password[position] == 0) {
                                uint8_t character = (digest[3] & 0xF0) >> 4;
                                if (character < 10) {
                                        password[position] = '0' + character;
                                } else {
                                        password[position] = 'a' + character - 10;
                                }
                                k++;
                        }
                }

                #ifdef DEBUG
                if (m++ % 10000 == 0) {
                        for (unsigned l=0; l<8; l++) {
                                char c = password[l];
                                if (c == 0) {
                                        c = rand() % 16;
                                        if (c < 10) {
                                                c += '0';
                                        } else {
                                                c += 'a' - 10;
                                        }
                                }
                                mvaddch(0, l, c);
                                refresh();
                        }
                }
                #endif
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", password);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
