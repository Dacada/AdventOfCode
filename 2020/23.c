#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TAGS 9
#define TAGS2 1000000

struct cup {
        int tag;
        struct cup *next;
        struct cup *prev;
};

struct cups {
        struct cup *current;
        struct cup *bytag;
};

static void parse(const char *input, struct cups *const cups, bool big) {
        static struct cup bytag[TAGS2+1];
        for (int i=0; i<=TAGS2; i++) {
                bytag[i].tag = i;
        }
        
        for (int i=0; i<TAGS; i++) {
                char c = input[i];
                ASSERT(isdigit(c), "parse error");
                int tag = c-'0';

                int previ = (i+TAGS-1)%TAGS;
                int prevtag = input[previ]-'0';

                int nexti = (i+1)%TAGS;
                int nexttag = input[nexti]-'0';

                bytag[tag].prev = bytag+prevtag;
                bytag[tag].next = bytag+nexttag;
        }
        
        bytag[0].tag = -1;
        bytag[0].next = NULL;
        bytag[0].prev = NULL;

        cups->current = bytag+input[0]-'0';
        cups->bytag = bytag;

        if (big) {
                for (int i=TAGS+1; i<=TAGS2; i++) {
                        bytag[i].prev = bytag+i-1;
                        bytag[i].next = bytag+i+1;
                }
                cups->current->prev->next = bytag+TAGS+1;
                bytag[TAGS+1].prev = cups->current->prev;
                cups->current->prev = bytag+TAGS2;
                bytag[TAGS2].next = cups->current;
        }
}

static void play_round(struct cups *const cups, const int tagmax) {
        struct cup *pickup_first = cups->current->next;
        struct cup *pickup_last = pickup_first->next->next;

        pickup_first->prev->next = pickup_last->next;
        pickup_last->next->prev = pickup_first->prev;
        //pickup_first->prev = NULL;
        //pickup_last->next = NULL;

        int dest_tag = cups->current->tag - 1;
        if (dest_tag == 0) {
                dest_tag = tagmax;
        }
        while (dest_tag == pickup_first->tag ||
               dest_tag == pickup_last->tag ||
               dest_tag == pickup_first->next->tag) {
                dest_tag--;
                if (dest_tag == 0) {
                        dest_tag = tagmax;
                }
        }
        struct cup *destination_cup = cups->bytag+dest_tag;

        destination_cup->next->prev = pickup_last;
        pickup_last->next = destination_cup->next;
        pickup_first->prev = destination_cup;
        destination_cup->next = pickup_first;

        cups->current = cups->current->next;
        if (cups->current->tag == -1) {
                cups->current = cups->current->next;
        }
}

static void solution1(const char *const input, char *const output) {
        struct cups cups;
        parse(input, &cups, false);

        #ifdef DEBUG
        {
                DBG("%s", input);
                struct cup *cup = cups.current;
                for(;;) {
                        DBG("%d -> %d", cup->tag, cup->next->tag);
                        cup = cup->next;
                        if (cup == cups.current) {
                                break;
                        }
                }
        }
        #endif

        for (int round=0; round<100; round++) {
                play_round(&cups, TAGS);

                #ifdef DEBUG
                struct cup *cup = cups.current;
                for(;;) {
                        fprintf(stderr, "%d ", cup->tag);
                        cup = cup->next;
                        if (cup == cups.current) {
                                fprintf(stderr, "\n");
                                break;
                        }
                }
                #endif
        }

        struct cup *cup = cups.bytag[1].next;
        char result[TAGS];
        for (int i=0; i<TAGS-1; i++) {
                result[i] = cup->tag + '0';
                cup = cup->next;
        }
        result[TAGS-1] = '\0';
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%s", result);
}

static void solution2(const char *const input, char *const output) {
        struct cups cups;
        parse(input, &cups, true);

        for (int round=0; round<10000000; round++) {
                play_round(&cups, TAGS2);
        }

        struct cup *cup1 = cups.bytag+1;
        long result = (long)cup1->next->tag * (long)cup1->next->next->tag;
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
