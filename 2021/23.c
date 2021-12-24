#include <aoclib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define MAXSTATES 128

static unsigned energies[4] = {1, 10, 100, 1000};
static unsigned edge2room[4][2] = {
        {0,6},
        {2,4},
        {4,2},
        {6,0},
};
static unsigned hall2room[4][3] = {
        {1,3,5},
        {1,1,3},
        {3,1,1},
        {5,3,1},
};

struct state {
        char edges[2][2];
        char hall[3];
        char rooms[4][4];
        unsigned energy;
        bool folded;
        unsigned count;
};

/* ############# */
/* #EE H H H EE# */
/* ###R#R#R#R### */
/*   #R#R#R#R#   */
/*   #########   */

/* ############# */
/* #10 0 1 2 01# */
/* ###0#0#0#0### */
/*   #1#1#1#1#   */
/*   #########   */

__attribute__((unused))
static void print_state(const struct state *const s) {
#ifdef DEBUG
        fprintf(stderr, "#############\n");
        fprintf(stderr, "#%c%c %c %c %c %c%c#\n", s->edges[0][1], s->edges[0][0], s->hall[0], s->hall[1], s->hall[2], s->edges[1][0], s->edges[1][1]);
        fprintf(stderr, "###%c#%c#%c#%c###\n", s->rooms[0][0], s->rooms[1][0], s->rooms[2][0], s->rooms[3][0]);
        fprintf(stderr, "  #%c#%c#%c#%c#  \n", s->rooms[0][1], s->rooms[1][1], s->rooms[2][1], s->rooms[3][1]);
        if (!s->folded) {
                fprintf(stderr, "  #%c#%c#%c#%c#  \n", s->rooms[0][2], s->rooms[1][2], s->rooms[2][2], s->rooms[3][2]);
                fprintf(stderr, "  #%c#%c#%c#%c#  \n", s->rooms[0][3], s->rooms[1][3], s->rooms[2][3], s->rooms[3][3]);
        }
        fprintf(stderr, "  #########  \n");
        fprintf(stderr, "count: %u\n", s->count);
        fprintf(stderr, "energy: %u\n\n", s->energy);
#else
        (void)s;
#endif
}


static inline bool isagent(const char c) {
        return c=='A'||c=='B'||c=='C'||c=='D';
}

static bool isorganized(const struct state *const s) {
        for (int i=0; i<4; i++) {
                int l;
                if (s->folded) {
                        l=2;  
                } else {
                        l=4;
                }
                for (int j=0; j<l; j++) {
                        if (s->rooms[i][j] != 'A'+i) {
                                return false;
                        }
                }
        }
        return true;
}

static void parse_input(const char *input, struct state *const state) {
#define VALIDATE(x) ASSERT(isagent(x), "parse error")
        
        while (*input != '\n') {
                input++;
        }
        input++;
        
        while (*input != '\n') {
                input++;
        }
        input++;

        input += 3;
        VALIDATE(*input);
        state->rooms[0][0] = *input;

        input += 2;
        VALIDATE(*input);
        state->rooms[1][0] = *input;

        input += 2;
        VALIDATE(*input);
        state->rooms[2][0] = *input;

        input += 2;
        VALIDATE(*input);
        state->rooms[3][0] = *input;
        
        while (*input != '\n') {
                input++;
        }
        input++;

        input += 3;
        VALIDATE(*input);
        state->rooms[0][1] = *input;

        input += 2;
        VALIDATE(*input);
        state->rooms[1][1] = *input;

        input += 2;
        VALIDATE(*input);
        state->rooms[2][1] = *input;

        input += 2;
        VALIDATE(*input);
        state->rooms[3][1] = *input;

        state->edges[0][0] = ' ';
        state->edges[0][1] = ' ';
        state->edges[1][0] = ' ';
        state->edges[1][1] = ' ';
        state->hall[0] = ' ';
        state->hall[1] = ' ';
        state->hall[2] = ' ';
        state->energy = 0;
        state->count = 0;
#undef VALIDATE
}

static void validate_state(const struct state *const s) {
        unsigned count[5]={0,0,0,0,0};
        for (int i=0; i<2; i++) {
                for (int j=0; j<2; j++) {               
                        char d = s->edges[i][j];      
                        if (d==' ') {                   
                                count[0]++;             
                        } else if (isagent(d)) {        
                                count[d-'A'+1]++;       
                        } else {                        
                                FAIL("invalid state");  
                        }                               
                }
        }                                       
        for (int j=0; j<4; j++) {
                int l;
                if (s->folded) {
                        l=2;  
                } else {
                        l=4;
                }
                for (int i=0; i<l; i++) {
                        char d = s->rooms[j][i];      
                        if (d==' ') {                   
                                count[0]++;             
                        } else if (isagent(d)) {        
                                count[d-'A'+1]++;       
                        } else {
                                FAIL("invalid state");
                        }
                }
        }
        for (int i=0; i<3; i++) {                       
                char d = s->hall[i];                  
                if (d==' ') {                           
                        count[0]++;                     
                } else if (isagent(d)) {                
                        count[d-'A'+1]++;               
                } else {                                
                        FAIL("invalid state");          
                }                                       
        }
        ASSERT(count[0]==7, "invalid state %d", count[0]);
        for (int i=1; i<=4; i++) {
                if (s->folded) {
                        ASSERT(count[i]==2, "invalid state");
                } else {
                        ASSERT(count[i]==4, "invalid state");
                }
        }
}

static bool edge2room_unblocked(const struct state *const state, int edge, int room) {
        
        bool h0 = isagent(state->hall[0]);
        bool h1 = isagent(state->hall[1]);
        bool h2 = isagent(state->hall[2]);

        bool e0 = edge==0;
        bool e1 = edge==1;
        ASSERT(e0||e1, "bad");

        bool r0 = room==0;
        bool r1 = room==1;
        bool r2 = room==2;
        bool r3 = room==3;
        ASSERT(r0||r1||r2||r3, "bad");

        return  (r0 && ((e0)                      || (e1 && !h0 && !h1 && !h2))) ||
                (r1 && ((e0 && !h0)               || (e1 && !h1 && !h2))) ||
                (r2 && ((e0 && !h0 && !h1)        || (e1 && !h2))) ||
                (r3 && ((e0 && !h0 && !h1 && !h2) || (e1)));
}

static bool hall2room_unblocked(const struct state *const state, int hall, int room) {
        bool h0 = isagent(state->hall[0]);
        bool h1 = isagent(state->hall[1]);
        bool h2 = isagent(state->hall[2]);

        bool c0 = hall==0;
        bool c1 = hall==1;
        bool c2 = hall==2;
        ASSERT(c0||c1||c2, "bad");

        bool r0 = room==0;
        bool r1 = room==1;
        bool r2 = room==2;
        bool r3 = room==3;
        ASSERT(r0||r1||r2||r3, "bad");

        return  (c0 && (r0                 || r1          || (r2 && !h1) || (r3 && !h1 && !h2))) ||
                (c1 && ((r0 && !h0)        || r1          || r2          || (r3 && !h2))) ||
                (c2 && ((r0 && !h0 && !h1) || (r1 && !h1) || r2          || r3));
}

static bool is_room_valid(const struct state *const state, const char *room, const char c) {
        if (state->folded) {
                return !isagent(room[0]) && (!isagent(room[1]) || room[1] == c);
        } else {
                return !isagent(room[0]) && (!isagent(room[1]) || room[1] == c) && (!isagent(room[2]) || room[2] == c) && (!isagent(room[3]) || room[3] == c);
        }
}

static int room_depth_enter(const struct state *const state, const char *const room) {
        if (state->folded) {
                if (!isagent(room[1])) {
                        return 1;
                } else {
                        return 0;
                }
        } else {
                for (int i=3; i>0; i--) {
                        if (!isagent(room[i])) {
                                return i;
                        }
                }
                return 0;
        }
}

static int room_depth_exit(const struct state *const state, const char *const room) {
        if (state->folded) {
                if (isagent(room[0])) {
                        return 0;
                } else {
                        return 1;
                }
        } else {
                for (int i=0; i<3; i++) {
                        if (isagent(room[i])) {
                                return i;
                        }
                }
                return 3;
        }
}

static bool should_exit(const struct state *const state, const char *const room, const char c, const int r) {
        if (c-'A' != r) {
                return true;
        }
        
        if (state->folded) {
                return room[1]-'A' != r;
        } else {
                for (int i=1; i<4; i++) {
                        if (room[i] != r) {
                                return true;
                        }
                }
                return false;
        }
}

static bool is_deadlock_state(const struct state *const state) {
        // Deadlock detection:
        //   1 A room has only one agent in it, c, which does not belong to the room.
        //   2 c cannot currently exit the room into any hallway or edge tiles.
        //   3 For rooms 0 and 3:
        //     3.1 The single hallway tile is occupied by an agent that belongs in c's current room, d
        //     3.2 d is also blocking the edge tile from moving to their rooms
        //   4 For rooms 1 and 2:
        //     4.1 The left hallway tile either belongs to c's room or to a room to the right of c's
        //     4.2 The right hallway tile either belongs to c's room or to a room to the left of c's
        char c;
        for (int r=0; r<4; r++) {
                if (state->folded) {
                        if (isagent(state->rooms[r][0]) || !isagent(state->rooms[r][1])) {
                                continue;
                        }
                        c = isagent(state->rooms[r][1]);
                } else {
                        if (isagent(state->rooms[r][0]) || isagent(state->rooms[r][1]) || isagent(state->rooms[r][2]) || !isagent(state->rooms[r][3])) {
                                continue;
                        }
                        c = isagent(state->rooms[r][3]);
                }

                // 1 Passes

                if (r==0 && (!isagent(state->edges[0][0]) || !isagent(state->hall[0]))) {
                        continue;
                }
                if (r==1 && (!isagent(state->hall[0]) || !isagent(state->hall[1]))) {
                        continue;
                }
                if (r==2 && (!isagent(state->hall[1]) || !isagent(state->hall[2]))) {
                        continue;
                }
                if (r==3 && (!isagent(state->edges[1][0]) || !isagent(state->hall[2]))) {
                        continue;
                }

                // 2 Passes

                if (r==0) {
                        if (state->hall[0] != 'A') {
                                continue;
                        }
                        if (state->edges[0][0] == 'A') {
                                continue;
                        }
                }
                if (r==3) {
                        if (state->hall[2] != 'D') {
                                continue;
                        }
                        if (state->edges[0][0] == 'D') {
                                continue;
                        }
                }

                if (r==1) {
                        if (state->hall[0] < c || state->hall[1] > c) {
                                continue;
                        }
                }
                if (r==2) {
                        if (state->hall[1] < c || state->hall[2] > c) {
                                continue;
                        }
                }

                // 3/4 pass

                //DBG("DEADLOCK!");
                //print_state(state);
                return true;
        }
        return false;
}

static size_t valid_states(const struct state *const state, struct state states[MAXSTATES]) {
#define NEXTSTATE                               \
        new=&states[s++];                       \
        *new=*state;                            \
        new->count++
#define ENERGY(x)                               \
        new->energy += (x) * energies[c-'A']

        if (is_deadlock_state(state)) {
                return 0;
        }

        size_t s = 0;
        struct state *new;
        char c;
        
        // Edges
        for (int edge=0; edge<2; edge++) {
                for (int i=0; i<2; i++) {
                        c = state->edges[edge][i];
                        if (isagent(c)) {
                                // Move to Room c
                                const char *room = state->rooms[c-'A'];
                                // Path to room must be unblocked
                                if (edge2room_unblocked(state, edge, c-'A')) {
                                        // Room must be unblocked and if occupied only by c
                                        if (is_room_valid(state, room, c)) {
                                                NEXTSTATE;
                                                new->edges[edge][i] = ' ';
                                                int ri = room_depth_enter(state, room);
                                                new->rooms[c-'A'][ri] = c;
                                                ENERGY(i+1+edge2room[c-'A'][edge]+1+ri);
                                                validate_state(new);
                                                if (is_deadlock_state(new)) {
                                                        s--;
                                                }
                                        }
                                }
                        }
                }
        }
        
        // Hallways
        for (int hall=0; hall<3; hall++) {
                c = state->hall[hall];
                if (isagent(c)) {
                        // Move to Room c
                        const char *room = state->rooms[c-'A'];
                        // Path to room must be unblocked
                        if (hall2room_unblocked(state, hall, c-'A')) {
                                // Room must be unblocked and if occupied only by c
                                if (is_room_valid(state, room, c)) {
                                        NEXTSTATE;
                                        new->hall[hall] = ' ';
                                        int ri = room_depth_enter(state, room);
                                        new->rooms[c-'A'][ri] = c;
                                        ENERGY(hall2room[c-'A'][hall]+1+ri);
                                        validate_state(new);
                                        if (is_deadlock_state(new)) {
                                                s--;
                                        }
                                }
                        }
                }
        }

        // Rooms
        for (int r=0; r<4; r++) {
                const char *room = state->rooms[r];
                int ri = room_depth_exit(state, room);
                
                c = room[ri];
                if (isagent(c)) {
                        // Move only if room is such that this agent should exit it
                        if (should_exit(state, room, c, r)) {
                                // Move to all unblocked hallways and edges
                                for (int edge=0; edge<2; edge++) {
                                        // Move to 0 or 1 if possible
                                        for (int i=0; i<2; i++) {
                                                if (!isagent(state->edges[edge][i])) {
                                                        if (edge2room_unblocked(state, edge, r)) {
                                                                NEXTSTATE;
                                                                new->rooms[r][ri] = ' ';
                                                                new->edges[edge][i] = c;
                                                                ENERGY(ri+1+edge2room[r][edge]+1+i);
                                                                validate_state(new);
                                                                if (is_deadlock_state(new)) {
                                                                        s--;
                                                                }
                                                        }
                                                }
                                        }
                                }
                                for (int hall=0; hall<3; hall++) {
                                        if (isagent(state->hall[hall])) {
                                                continue;
                                        }
                                        if (hall2room_unblocked(state, hall, r)) {
                                                NEXTSTATE;
                                                new->rooms[r][ri] = ' ';
                                                new->hall[hall] = c;
                                                ENERGY(ri+1+hall2room[r][hall]);
                                                validate_state(new);
                                                if (is_deadlock_state(new)) {
                                                        s--;
                                                }
                                        }
                                }
                        }
                }
        }

        return s;
#undef ENERGY
#undef NEXTSTATE
}

struct queue {
        struct state *nodes;
        size_t size;
        size_t len;
};

static void queue_realloc(struct queue *const q) {
        q->nodes = realloc(q->nodes, q->size * sizeof(*q->nodes));
}

static void queue_init(struct queue *const q) {
        q->len = 0;
        q->size = 64;
        q->nodes = NULL;
        queue_realloc(q);
}

static bool queue_empty(struct queue *const q) {
        return q->len == 0;
}

static void queue_push(struct queue *const q, const struct state *const s) {
        if (q->len >= q->size) {
                q->size *= 2;
                queue_realloc(q);
        }
        
        q->nodes[q->len] = *s;

        size_t i = q->len;
        size_t p_i = (i-1)/2;

        for (;;) {
                if (i == 0) {
                        break;
                }
                if (q->nodes[p_i].energy < q->nodes[i].energy) {
                        break;
                }
                
                struct state tmp = q->nodes[i];
                q->nodes[i] = q->nodes[p_i];
                q->nodes[p_i] = tmp;

                i = p_i;
                p_i = (i-1)/2;
        }
        
        q->len += 1;



        
        /* q->nodes[q->len] = *s; */
        /* q->len += 1; */
}

static void queue_pop(struct queue *const q, struct state *const s) {
        ASSERT(q->len > 0, "pop empty queue");



        
        q->len -= 1;
        *s = q->nodes[0];
        q->nodes[0] = q->nodes[q->len];
        
        size_t i = 0;
        size_t c1_i = 2*i+1;
        size_t c2_i = 2*i+2;

        for (;;) {
                if (c1_i >= q->len) {
                        break;
                }
                size_t c_i;
                if (c2_i >= q->len) {
                        c_i = c1_i;
                } else {
                        if (q->nodes[c1_i].energy < q->nodes[c2_i].energy) {
                                c_i = c1_i;
                        } else {
                                c_i = c2_i;
                        }
                }
                if (q->nodes[i].energy < q->nodes[c_i].energy) {
                        break;
                }

                struct state tmp = q->nodes[i];
                q->nodes[i] = q->nodes[c_i];
                q->nodes[c_i] = tmp;

                i = c_i;
                c1_i = 2*i+1;
                c2_i = 2*i+2;
        }



        
        /* q->len -= 1; */
        /* *s = q->nodes[q->len]; */
}

static void queue_free(struct queue *const q) {
        free(q->nodes);
}

#define SET_STR_LEN 23

struct setNode {
        char str[SET_STR_LEN];
        unsigned val;
        struct setNode *left;
        struct setNode *right;
};

static struct setNode *setNode_new(const char *const str, unsigned val) {
        struct setNode *n = malloc(sizeof(*n));
        strncpy(n->str, str, SET_STR_LEN);
        n->val = val;
        n->left = NULL;
        n->right = NULL;
        return n;
}

static void setNode_free(struct setNode *const n) {
        if (n == NULL) {
                return;
        }
        setNode_free(n->left);
        setNode_free(n->right);
        free(n);
}

static bool setNode_add(struct setNode *const n, const char *const str, unsigned val) {
        int cmp = strncmp(str, n->str, SET_STR_LEN);
        if (cmp == 0) {
                if (n->val > val) {
                        n->val = val;
                        return true;
                } else {
                        return false;
                }
        } else if (cmp < 0) {
                if (n->left == NULL) {
                        n->left = setNode_new(str, val);
                        return true;
                } else {
                        return setNode_add(n->left, str, val);
                }
        } else if (cmp > 0) {
                if (n->right == NULL) {
                        n->right = setNode_new(str, val);
                        return true;
                } else {
                        return setNode_add(n->right, str, val);
                }
        } else {
                FAIL("impossible");
        }
}

struct set {
        struct setNode *root;
};

static void set_init(struct set *const h) {
        h->root = NULL;
}

// return false if already in set and can't replace
static bool set_add(struct set *const h, const struct state *const s) {
        char str[SET_STR_LEN];
        int ss = 0;
        for (int i=0; i<2; i++) {
                for (int j=0; j<2; j++) {
                        str[ss++] = s->edges[i][j];
                }
        }
        for (int i=0; i<3; i++) {
                str[ss++] = s->hall[i];
        }
        for (int i=0; i<4; i++) {
                int l;
                if (s->folded) {
                        l = 2;
                } else {
                        l = 4;
                }
                for (int j=0; j<l; j++) {
                        str[ss++] = s->rooms[i][j];
                }
        }

        if (h->root == NULL) {
                h->root = setNode_new(str, s->energy);
                return true;
        } else {
                return setNode_add(h->root, str, s->energy);
        }
}

static void set_free(struct set *const h) {
        setNode_free(h->root);
}

static unsigned search(const struct state *const start) {
        static struct queue q;
        queue_init(&q);
        queue_push(&q, start);

        static struct set st;
        set_init(&st);

        unsigned best = UINT_MAX;
        while (!queue_empty(&q)) {
                struct state s;
                queue_pop(&q, &s);
                //print_state(&s);
                if (s.energy >= best) {
                        continue;
                }
                if (!set_add(&st, &s)) {
                        continue;
                }


                static struct state next[MAXSTATES];
                size_t l = valid_states(&s, next);
                if (l==0) {
                        //DBG("does not continue");
                }
                for (size_t i=0; i<l; i++) {
                        if (isorganized(&next[i])) {
                                if (next[i].energy < best) {
                                        print_state(&s);
                                        DBG("%u", next[i].energy);
                                        best = next[i].energy;
                                }
                        } else {
                                queue_push(&q, &next[i]);
                        }
                }
        }

        set_free(&st);
        queue_free(&q);
        return best;
}

static void solution1(const char *const input, char *const output) {
        struct state start;
        parse_input(input, &start);
        
        start.folded = true;
        
        unsigned result = search(&start);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

static void solution2(const char *const input, char *const output) {
        struct state start;
        parse_input(input, &start);
        
        start.folded = false;
        for (int i=0; i<4; i++) {
                start.rooms[i][3] = start.rooms[i][1];
        }
        start.rooms[0][1] = 'D';
        start.rooms[1][1] = 'C';
        start.rooms[2][1] = 'B';
        start.rooms[3][1] = 'A';
        start.rooms[0][2] = 'D';
        start.rooms[1][2] = 'B';
        start.rooms[2][2] = 'A';
        start.rooms[3][2] = 'C';
        
        unsigned result = search(&start);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
