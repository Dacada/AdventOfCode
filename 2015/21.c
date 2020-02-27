#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>

struct item {
        unsigned cost, damage, armor;
};

struct character {
        unsigned hp, damage, armor;
};

static struct item weapons[] = {
        { .cost=8,   .damage=4, .armor=0 }, // Dagger
        { .cost=10,  .damage=5, .armor=0 }, // Shortsword
        { .cost=25,  .damage=6, .armor=0 }, // Warhammer
        { .cost=40,  .damage=7, .armor=0 }, // Longsword
        { .cost=74,  .damage=8, .armor=0 }, // Greataxe
};

static struct item armors[] = {
        { .cost=0,  .damage=0, .armor=0 }, // Naked
        { .cost=13,  .damage=0, .armor=1 }, // Leather
        { .cost=31,  .damage=0, .armor=2 }, // Chainmail
        { .cost=53,  .damage=0, .armor=3 }, // Splintmail
        { .cost=75,  .damage=0, .armor=4 }, // Bandedmail
        { .cost=102, .damage=0, .armor=5 }, // Platemail
};

static struct item rings[] = {
        { .cost=0,  .damage=0, .armor=0 }, // No ring
        { .cost=25,  .damage=1, .armor=0 }, // Damage +1
        { .cost=50,  .damage=2, .armor=0 }, // Damage +2
        { .cost=100, .damage=3, .armor=0 }, // Damage +3
        { .cost=20,  .damage=0, .armor=1 }, // Defense +1
        { .cost=40,  .damage=0, .armor=2 }, // Defense +2
        { .cost=80,  .damage=0, .armor=3 }, // Defense +3
};

static unsigned parse_number(const char **const input) {
        unsigned n = 0;
        while (isspace(**input)) (*input)++;
        while (**input >= '0' && **input <= '9') {
                n = n * 10 + **input - '0';
                (*input)++;
        }
        return n;
}

static void parse_input(const char *input, struct character *const boss) {
        while (*input != ':') input++;
        input++;
        boss->hp = parse_number(&input);
        
        while (*input != ':') input++;
        input++;
        boss->damage = parse_number(&input);
        
        while (*input != ':') input++;
        input++;
        boss->armor = parse_number(&input);
}

static unsigned get_net_dmg(unsigned damage, unsigned armor) {
        if (armor >= damage) {
                return 1;
        } else {
                return damage - armor;
        }
}

static bool victory(const struct character *const c1, const struct character *const c2) {
        unsigned net_dmg_1 = get_net_dmg(c1->damage, c2->armor);
        unsigned net_dmg_2 = get_net_dmg(c2->damage, c1->armor);

        unsigned turns_win_1 = (c2->hp + net_dmg_1 - 1) / net_dmg_1; // ceil divison
        unsigned turns_win_2 = (c1->hp + net_dmg_2 - 1) / net_dmg_2;

        return turns_win_1 <= turns_win_2; // c1 goes first
}

static unsigned get_cost(unsigned w, unsigned a, unsigned r1, unsigned r2) {
        return weapons[w].cost + armors[a].cost + rings[r1].cost + rings[r2].cost;
}

static void equip(struct character *const c, unsigned w, unsigned a, unsigned r1, unsigned r2) {
        c->armor += weapons[w].armor + armors[a].armor + rings[r1].armor + rings[r2].armor;
        c->damage += weapons[w].damage + armors[a].damage + rings[r1].damage + rings[r2].damage;
}

static void unequip(struct character *const c, unsigned w, unsigned a, unsigned r1, unsigned r2) {
        c->armor -= weapons[w].armor + armors[a].armor + rings[r1].armor + rings[r2].armor;
        c->damage -= weapons[w].damage + armors[a].damage + rings[r1].damage + rings[r2].damage;
}

static void solution(const char *const input, char *const output, bool minimize, bool win) {
        struct character boss;
        parse_input(input, &boss);

        struct character player;
        player.hp = 100;
        player.damage = 0;
        player.armor = 0;

        size_t weapons_len = (sizeof(weapons)/sizeof(weapons[0]));
        size_t armors_len = (sizeof(armors)/sizeof(armors[0]));
        size_t rings_len = (sizeof(rings)/sizeof(rings[0]));

        unsigned best_cost = minimize ? UINT_MAX : 0;
        
        for (size_t weapon=0; weapon<weapons_len; weapon++) {
                for (size_t armor=0; armor<armors_len; armor++) {
                        for (size_t ring1=0; ring1<rings_len; ring1++) {
                                for (size_t ring2=0; ring2<rings_len; ring2++) {
                                        if ((ring1 == 0 && ring2 == 0) || (ring1 != ring2)) {
                                                equip(&player, weapon, armor, ring1, ring2);
                                                bool vic = victory(&player, &boss);
                                                if ((win && vic) || (!win && !vic)) {
                                                        unsigned cost = get_cost(weapon, armor, ring1, ring2);
                                                        if ((minimize && cost < best_cost) ||
                                                            (!minimize && cost > best_cost)) {
                                                                best_cost = cost;
                                                        }
                                                }
                                                unequip(&player, weapon, armor, ring1, ring2);
                                        }
                                }
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%u", best_cost);
}

static void solution1(const char *const input, char *const output) {
        solution(input, output, true, true);
}

static void solution2(const char *const input, char *const output) {
        solution(input, output, false, false);}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
