#define _POSIX_C_SOURCE 200809L

#include <aoclib.h>
#include <bsd/md5.h>
#include <stdio.h>
#include <string.h>

#define QSIZE (1<<16)

static size_t strlen_noeol(const char *const input) {
        for (size_t i=0;; i++) {
                if (input[i] == '\0' || input[i] == '\n') {
                        return i;
                }
        }
}

struct state {
        MD5_CTX hash;
        int x,y;
        char *path;
        size_t pathlen;
};

static void sinit(struct state *const s, const char *const input) {
        MD5Init(&s->hash);
        MD5Update(&s->hash, (const uint8_t *)input, strlen_noeol(input));
        s->x = 0;
        s->y = 3;
        s->pathlen = 0;
        s->path = malloc(sizeof(char)*s->pathlen);
}

struct queue {
        struct state states[QSIZE];
        size_t head, tail;
};

static void qinit(struct queue *const q) {
        q->head = q->tail = 0;
}

static bool qempty(struct queue *const q) {
        return q->tail == q->head;
}

static struct state *qpush(struct queue *const q) {
        struct state *s = &q->states[q->tail];
        q->tail++;
        q->tail%=QSIZE;
        ASSERT(!qempty(q), "queue full");
        return s;
}

static void qunpush(struct queue *const q) {
        ASSERT(!qempty(q), "queue empty");
        q->tail--;
        q->tail%=QSIZE;
}

static struct state *qpop(struct queue *const q) {
        ASSERT(!qempty(q), "queue empty");
        struct state *s = &q->states[q->head];
        q->head++;
        q->head%=QSIZE;
        return s;
}

static bool hasdoor(int dir, int x, int y) {
        return (x!=0 || dir!=2) && (x!=3 || dir!=3) && (y!=0 || dir!=1) && (y!=3 || dir!=0);
}

static bool isopen(int dir, uint8_t d1, uint8_t d2) {
        return (((((uint16_t)d1 << 8) | d2) >> ((3-dir)*4)) & 0xF) > 0xA;
}

static bool validdir(int dir, int x, int y, uint8_t d1, uint8_t d2) {
        return hasdoor(dir, x, y) && isopen(dir, d1, d2);
}

static char dirupdate(int dir, struct state *const s) {
        switch (dir) {
        case 0:
                s->y++;
                ASSERT(s->y < 4, "y overrun");
                return 'U';
        case 1:
                s->y--;
                ASSERT(s->y >= 0, "y underrun");
                return 'D';
        case 2:
                s->x--;
                ASSERT(s->x < 4, "x overrun");
                return 'L';
        case 3:
                s->x++;
                ASSERT(s->x >= 0, "x underrun");
                return 'R';
        default:
                FAIL("unexpected dir");
        }
}

static void copy_state_and_get_digest(const struct state *const src, struct state *const dst, uint8_t *digest) {
        *dst = *src;
        MD5Final(digest, &dst->hash);
        dst->hash = src->hash;
}

static void update_path(struct state *const current, const struct state *const previous, char direction) {
        current->path = malloc((previous->pathlen+1)*sizeof(char));
        strncpy(current->path, previous->path, previous->pathlen);
        current->path[previous->pathlen] = direction;
        current->pathlen = previous->pathlen + 1;
        MD5Update(&current->hash, (uint8_t*)&direction, 1);
}

static void solution1(const char *const input, char *const output) {
        struct queue q;
        qinit(&q);
        
        struct state *s = qpush(&q);
        sinit(s, input);

        bool found = false;
        while (!qempty(&q)) {
                s = qpop(&q);
                
                struct state prev_s;
                uint8_t digest[MD5_DIGEST_LENGTH];
                copy_state_and_get_digest(s, &prev_s, digest);
                
                for (int dir=0; dir<4; dir++) {
                        if (validdir(dir, prev_s.x, prev_s.y, digest[0], digest[1])) {
                                s = qpush(&q);
                                *s = prev_s;
                                
                                char cdir = dirupdate(dir, s);
                                update_path(s, &prev_s, cdir);

                                if (s->x == 3 && s->y == 0) {
                                        found = true;
                                        ASSERT(s->pathlen+1 < OUTPUT_BUFFER_SIZE, "path too long");
                                        strncpy(output, s->path, s->pathlen);
                                        output[s->pathlen] = '\0';
                                        free(prev_s.path);
                                        goto end;
                                }
                        }
                }

                // free old state
                free(prev_s.path);
        }

end:
        ASSERT(found, "not found");
        // free all states in queue
        while (!qempty(&q)) {
                s = qpop(&q);
                free(s->path);
        }
}

static void solution2(const char *const input, char *const output) {
        struct queue q;
        qinit(&q);
        
        struct state *s = qpush(&q);
        sinit(s, input);

        size_t longest = 0;
        while (!qempty(&q)) {
                s = qpop(&q);

                struct state prev_s;
                uint8_t digest[MD5_DIGEST_LENGTH];
                copy_state_and_get_digest(s, &prev_s, digest);

                for (int dir=0; dir<4; dir++) {
                        if (validdir(dir, prev_s.x, prev_s.y, digest[0], digest[1])) {
                                s = qpush(&q);
                                *s = prev_s;
                                
                                char cdir = dirupdate(dir, s);
                                update_path(s, &prev_s, cdir);

                                if (s->x == 3 && s->y == 0) {
                                        if (s->pathlen > longest) {
                                                longest = s->pathlen;
                                        }
                                        free(s->path);
                                        qunpush(&q);
                                }
                        }
                }

                free(prev_s.path);
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", longest);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
