#define _POSIX_C_SOURCE 200809L

#include "intcode.h"
#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>

#define BUFFSIZE (1<<6)
#define CHECKPOINT "Security Checkpoint"
#define NBANNEDITEMS 5
#define NROOMS 19

struct room {
        struct room *north, *south, *west, *east;
        unsigned times_visited;
        bool parsed, checkpoint;
};

static void init_room(struct room *const r) {
        r->parsed = false;
        r->times_visited = 0;
        r->north = r->south = r->west = r->east = NULL;
}

static size_t next_room_from_pool = 0;
static struct room pool_of_rooms[BUFFSIZE];
static struct room *alloc_room(void) {
        struct room *r = pool_of_rooms + next_room_from_pool;
        next_room_from_pool++;
        ASSERT(next_room_from_pool < BUFFSIZE, "out of memory :c");
        return r;
}

static void handle_banned_item(char *const item) {
        static const char *const banned_items[NBANNEDITEMS] = {
                "giant electromagnet",
                "escape pod",
                "infinite loop",
                "photons",
                "molten lava"
        };

        for (size_t i=0; i<NBANNEDITEMS; i++) {
                if (strcmp(item, banned_items[i]) == 0) {
                        item[0] = '\0';
                        return;
                }
        }
}

static char *issue_command(struct IntCodeMachine *machine, const char *const command, bool get_text) {
        ASSERT(machine_send_input_string(machine, command), "machine did not take input");
#ifdef DEBUG
        fputs(command, stderr);
#endif
        
#ifdef DEBUG
        char *str = machine_recv_output_string(machine);
        ASSERT(str != NULL, "machine did not send output");
        fputs(str, stderr);
        if (get_text) {
                return str;
        } else {
                free(str);
                return NULL;
        }
#else
        if (get_text) {
                return machine_recv_output_string(machine);
        } else {
                machine_discard_output_string(machine);
                return NULL;
        }
#endif
}

static void parse_prompt(const char *const prompt, struct room *const room, char item[BUFFSIZE]) {
        if (room->parsed) {
                return;
        }

#ifdef DEBUG
        const char *const f = "prompt parse error '%c'";
#endif
        
        size_t i=0;
        
        while(isspace(prompt[i++])){} i--;
        ASSERT(prompt[i] == '=', f, prompt[i]); i++;
        ASSERT(prompt[i] == '=', f, prompt[i]); i++;
        ASSERT(prompt[i] == ' ', f, prompt[i]); i++;
        if (strncmp(prompt+i, CHECKPOINT, strlen(CHECKPOINT)) == 0) {
                room->checkpoint = true;
        } else {
                room->checkpoint = false;
        }
                
        while (prompt[i++] != '\n');

        // Skipped through room name

        ASSERT(isupper(prompt[i]), f, prompt[i]); i++;        
        while (prompt[i++] != '\n');
        ASSERT(prompt[i] == '\n', f, prompt[i]); i++;

        // Skipped through description

        ASSERT(prompt[i] == 'D', f, prompt[i]); // "Doors here lead:"
        i += 17;

        bool nswe[4] = {false, false, false, false};
        do {
                ASSERT(prompt[i] == '-', f, prompt[i]); i++; // Dash of item listing
                ASSERT(prompt[i] == ' ', f, prompt[i]); i++; // Space after dash
                switch (prompt[i]) {
                case 'n':
                        nswe[0] = true;
                        i += 6;
                        break;
                case 's':
                        nswe[1] = true;
                        i += 6;
                        break;
                case 'w':
                        nswe[2] = true;
                        i += 5;
                        break;
                case 'e':
                        nswe[3] = true;
                        i += 5;
                        break;
                default:
                        FAIL(f, prompt[i]);
                }
        } while (prompt[i] != '\n');
        i++;

        // Skipped through doors (expecting at most 4 per room)

        int j = 0;
        if (prompt[i] == 'I') { // Items here:
                i += 12;
                ASSERT(prompt[i] == '-', f, prompt[i]); i++; // Dash of item listing
                ASSERT(prompt[i] == ' ', f, prompt[i]); i++; // Space after dash
                while(prompt[i+(j++)] != '\n');
                j--;
                strncpy(item, prompt+i, j);
        }
        item[j] = '\0';
        handle_banned_item(item);

        // Skipped through items (expecting only one per room)

        if (nswe[0] && room->north == NULL) {
                struct room *r = alloc_room();
                init_room(r);
                room->north = r;
                r->south = room;
        }

        if (nswe[1] && room->south == NULL) {
                struct room *r = alloc_room();
                init_room(r);
                room->south = r;
                r->north = room;
        }

        if (nswe[2] && room->west == NULL) {
                struct room *r = alloc_room();
                init_room(r);
                room->west = r;
                r->east = room;
        }

        if (nswe[3] && room->east == NULL) {
                struct room *r = alloc_room();
                init_room(r);
                room->east = r;
                r->west = room;
        }
        
        room->parsed = true;
}

static struct room *decide_explore_direction(const struct room *const room, const char **const direction) {
        struct room *best = NULL;

        int count_unvisited = 0;;
        for (int i=0; i<4; i++) {
                struct room *next;
                const char *d;
                switch (i) {
                case 0: next = room->north; d="north"; break;
                case 1: next = room->south; d="south"; break;
                case 2: next = room->west;  d="west";  break;
                case 3: next = room->east;  d="east";  break;
                default: FAIL("excuse me, what the fuck?");
                }

                if (next == NULL) {
                        continue;
                }

                if (best == NULL) {
                        best = next;
                        *direction = d;
                }

                if (best->times_visited > next->times_visited) {
                        best = next;
                        *direction = d;
                }

                if (next->times_visited == 0) {
                        count_unvisited++;
                }
        }

        return best;
}

static struct room *explore_and_recover_all_items(struct IntCodeMachine *machine) {
        int count = 0;
        int items = 0;
        struct room *room = alloc_room();
        struct room *prev = NULL;
        init_room(room);

        while (count < NROOMS) {
                room->times_visited++;
                if (!room->parsed) {
                        count++;
                }
                
                char *prompt = machine_recv_output_string(machine);
                ASSERT(prompt != NULL, "machine produced no output");
#ifdef DEBUG
                fputs(prompt, stderr);
                fputs("> ", stderr);
#endif
                ASSERT(machine->running, "machine stopped running?");

                static char item[BUFFSIZE];
                item[0] = '\0';
                parse_prompt(prompt, room, item);
                free(prompt);

                if (room->checkpoint) {
                        const char *d;
                        if (room->north == prev) {
                                d = "north\n";
                        } else if (room->south == prev) {
                                d = "south\n";
                        } else if (room->east == prev) {
                                d = "east\n";
                        } else if (room->west == prev) {
                                d = "west\n";
                        } else {
                                FAIL("fuck");
                        }
                        ASSERT(machine_send_input_string(machine, d), "machine accepted no input");
#ifdef DEBUG
                        fputs(d, stderr);
#endif
                        room->times_visited = 99;
                        room = prev;
                        continue;
                }

                if (item[0] != '\0') {
                        items++;
                        ASSERT(machine_send_input_string(machine, "take "), "machine accepted no input");
                        ASSERT(machine_send_input_string(machine, item), "machine accepted no input");
                        ASSERT(machine_send_input_string(machine, "\n"), "machine accepted no input");
#ifdef DEBUG
                        fputs("take ", stderr);
                        fputs(item, stderr);
                        fputs("\n", stderr);
#endif
                        ASSERT(machine->running, "machine stopped running?");

#ifdef DEBUG
                        char *confirmation = machine_recv_output_string(machine);
                        fputs(confirmation, stderr);
                        free(confirmation);
                        fputs("> ", stderr);
#else
                        machine_discard_output_string(machine);
#endif
                }

                const char *dir;
                struct room *next = decide_explore_direction(room, &dir);
                ASSERT(machine_send_input_string(machine, dir), "machine accepted no input");
                ASSERT(machine_send_input_string(machine, "\n"), "machine accepted no input");
#ifdef DEBUG
                fputs(dir, stderr);
#endif
                ASSERT(machine->running, "machine stopped running?");

                prev = room;
                room = next;
        }
        
#ifdef DEBUG
        char *str = machine_recv_output_string(machine);
        ASSERT(str != NULL, "machine did not send output");
        fputs(str, stderr);
        free(str);
#else
        machine_discard_output_string(machine);
#endif
        return room;
}

static struct room *find_route_to_checkpoint(struct room *start, int *const route) {
        start->times_visited++;
        if (start->checkpoint) {
                *route = 0;
                return start;
        }
        
        if (start->north != NULL && !start->north->times_visited) {
                route[0] = 1;
                struct room *end = find_route_to_checkpoint(start->north, route+1);
                if (end != NULL) {
                        return end;
                }
        }
        
        if (start->south != NULL && !start->south->times_visited) {
                route[0] = 2;
                struct room *end = find_route_to_checkpoint(start->south, route+1);
                if (end != NULL) {
                        return end;
                }
        }
        
        if (start->east != NULL && !start->east->times_visited) {
                route[0] = 3;
                struct room *end = find_route_to_checkpoint(start->east, route+1);
                if (end != NULL) {
                        return end;
                }
        }
        
        if (start->west != NULL && !start->west->times_visited) {
                route[0] = 4;
                struct room *end = find_route_to_checkpoint(start->west, route+1);
                if (end != NULL) {
                        return end;
                }
        }

        return NULL;
}

static struct room *go_to_checkpoint(struct IntCodeMachine *machine, struct room *room) {
        int route[NROOMS];
        room = find_route_to_checkpoint(room, route);
        for (int i=0; i<NROOMS; i++) {
                int r = route[i];
                const char *direction;
                if (r == 0) {
                        break;
                } else if (r == 1) {
                        direction = "north\n";
                } else if (r == 2) {
                        direction = "south\n";
                } else if (r == 3) {
                        direction = "east\n";
                } else if (r == 4) {
                        direction = "west\n";
                } else {
                        FAIL("nope");
                }
                issue_command(machine, direction, false);
        }
        return room;
}

static void clean_visited_counter(struct room *room) {
        room->times_visited = 0;
        if (room->north != NULL && room->north->times_visited) clean_visited_counter(room->north);
        if (room->south != NULL && room->south->times_visited) clean_visited_counter(room->south);
        if (room->east != NULL && room->east->times_visited) clean_visited_counter(room->east);
        if (room->west != NULL && room->west->times_visited) clean_visited_counter(room->west);
}

static void get_item_names(struct IntCodeMachine *machine, char *items[8]) {
#ifdef DEBUG
        const char *const f = "inv parse error '%c'";
#endif
        
        char *text = issue_command(machine, "inv\n", true);

        int i=0;
        while (text[i++]=='\n'){} i--;
        ASSERT(text[i] == 'I', f, text[i]);
        while (text[i++]!='\n');
        
        int k=0;
        do {
                ASSERT(text[i] == '-', f, text[i]);
                i+=2;
                int j=0;
                while(text[i+(j++)] != '\n');
                j--;
                items[k++] = strndup(text+i, j);
                i += j+1;
        } while(text[i] != '\n');

        free(text);
}

static long parse_pressure(const char *const text) {
#ifdef DEBUG
        const char *const f = "failed to parse '%c'";
#endif
        
        size_t i=0;

        while (isspace(text[i++])){} i--;
        ASSERT(text[i] == '=', f, text[i]);
        
        while (text[i++] != '\n');
        ASSERT(text[i] == 'A', f, text[i]);
        
        while (text[i++] != '\n');
        ASSERT(text[i] == '\n', f, text[i]);
        i++;
        ASSERT(text[i] == 'D', f, text[i]);

        while (text[i++] != '\n');
        ASSERT(text[i] == '-', f, text[i]);
        
        while (text[i++] != '\n');
        ASSERT(text[i] == '\n', f, text[i]);
        i++;

        ASSERT(text[i] == 'A', f, text[i]);
        i += 59;
        
        if (text[i] == 'l' || text[i] == 'h') {
                return -1;
        } else {
                while (text[i++] != '\n');
                while (text[i++] != '\n');
                i += 51;
                ASSERT(isdigit(text[i]), "oof: %c", text[i]);
                
                long result = 0;
                while (isdigit(text[i])) {
                        result = result * 10 + text[i] - '0';
                        i++;
                }
                return result;
        }
}

struct find_item_combination_cbargs {
        struct IntCodeMachine *machine; //in
        char **item_names; //in
        long password; //inout
        int size; //in
        const char *go; //in
};
static void find_item_combination_cb(int *nums, void *vargs) {
        struct find_item_combination_cbargs *args = vargs;
        if (args->password != -1) {
                return;
        }
        
        for (int i=0; i<args->size; i++) {
                char *item = args->item_names[nums[i]];
                ASSERT(machine_send_input_string(args->machine, "drop "), "machine did not take input");
                ASSERT(machine_send_input_string(args->machine, item), "machine did not take input");
#ifdef DEBUG
                fputs("drop ", stderr);
                fputs(item, stderr);
#endif
                issue_command(args->machine, "\n", false);
        }
        
        char *text = issue_command(args->machine, args->go, true);
        args->password = parse_pressure(text);
        free(text);
        if (args->password != -1) {
                return;
        }

        for (int i=0; i<args->size; i++) {
                char *item = args->item_names[nums[i]];
                ASSERT(machine_send_input_string(args->machine, "take "), "machine did not take input");
                ASSERT(machine_send_input_string(args->machine, item), "machine did not take input");
#ifdef DEBUG
                fputs("take ", stderr);
                fputs(item, stderr);
#endif
                issue_command(args->machine, "\n", false);
        }
}


static long find_item_combination(struct IntCodeMachine *machine,
                                  char *item_names[8], struct room *room) {
        const char *go = NULL;
        if (room->north != NULL && !room->north->parsed) {
                go = "north\n";
        }
        if (room->south != NULL && !room->south->parsed) {
                go = "south\n";
        }
        if (room->west != NULL && !room->west->parsed) {
                go = "west\n";
        }
        if (room->east != NULL && !room->east->parsed) {
                go = "east\n";
        }

        int nums[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
        struct find_item_combination_cbargs args;
        args.item_names = item_names;
        args.machine = machine;
        args.go = go;
        args.password = -1;
        
        for (int i=1; i<8; i++) {
                args.size = i;
                aoc_combinations(nums, 8, i, find_item_combination_cb, &args);
                if (args.password != -1) {
                        break;
                }
        }
        
        return args.password;
}


static void solution1(const char *const input, char *const output) {
        struct IntCodeMachine machine;
        machine_init(&machine, input);
        machine_run(&machine);

        struct room *room = explore_and_recover_all_items(&machine);
        clean_visited_counter(room);
        room = go_to_checkpoint(&machine, room);
        ASSERT(room->checkpoint, "oops");
        clean_visited_counter(room);

        char *items[8];
        get_item_names(&machine, items);
        long result = find_item_combination(&machine, items, room);

        for (int i=0; i<8; i++) {
               free(items[i]);
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
        machine_free(&machine);
}

static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "Santa is safe once again!");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
