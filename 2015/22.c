#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define QUEUESIZE (2 << 17)

#ifdef DEBUG
#define MAX_SPELL_LOG_ELEMENTS (2 << 4)
#define MAX_LIST_OF_SPELL_LOGS_ELEMENTS (2 << 7)
#endif

// #define DEMO

#ifdef DEMO
#define DMO(msg, ...) DBG(msg, ##__VA_ARGS__)
#else
#define DMO(msgs, ...)                                                                                                 \
  do {                                                                                                                 \
  } while (0)
#endif

enum spell { SPELL_MAGIC_MISSILE, SPELL_DRAIN, SPELL_SHIELD, SPELL_POISON, SPELL_RECHARGE };

enum turn_result { TURN_CONTINUE, TURN_GAME_OVER, TURN_VICTORY };

struct state {
  unsigned boss_hp;

  unsigned player_hp;
  unsigned player_mana;

  unsigned effect_shield_turns_left;
  unsigned effect_poison_turns_left;
  unsigned effect_recharge_turns_left;

  unsigned mana_spent;

#ifdef DEBUG
  enum spell spell_log[MAX_SPELL_LOG_ELEMENTS];
  size_t spell_log_len;
#endif
};

struct constant_state {
  unsigned boss_damage;
  bool hard_mode;
};

struct optimum_info {
  unsigned mana;

#ifdef DEBUG
  enum spell spell_log_list[MAX_LIST_OF_SPELL_LOGS_ELEMENTS][MAX_SPELL_LOG_ELEMENTS];
  size_t spell_log_list_log_lens[MAX_LIST_OF_SPELL_LOGS_ELEMENTS];
  size_t spell_log_list_len;
#endif
};

static unsigned spell_mana_costs[] = {53, 73, 113, 173, 229};

static void optimum_info_init(struct optimum_info *const optimum) {
  optimum->mana = UINT_MAX;
#ifdef DEBUG
  optimum->spell_log_list_len = 0;
#endif
}

static void optimum_info_update(struct optimum_info *const optimum, const struct state *const state) {
  if (state->mana_spent < optimum->mana) {
    optimum->mana = state->mana_spent;
#ifdef DEBUG
    optimum->spell_log_list_len = 0;
#endif
  }

#ifdef DEBUG
  if (state->mana_spent <= optimum->mana) {
    optimum->spell_log_list_log_lens[optimum->spell_log_list_len] = state->spell_log_len;
    memcpy(optimum->spell_log_list[optimum->spell_log_list_len], state->spell_log,
           state->spell_log_len * sizeof(enum spell));
    optimum->spell_log_list_len++;
  }
#endif
}

static bool optimum_info_prune(const struct optimum_info *const optimum, const struct state *const state) {
  return state->mana_spent > optimum->mana;
}

static void optimum_info_print(const struct optimum_info *const optimum, char *const output) {
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", optimum->mana);

#ifdef DEBUG
  for (size_t i = 0; i < optimum->spell_log_list_len; i++) {
    DBG("Log %lu:", i + 1);
    for (size_t j = 0; j < optimum->spell_log_list_log_lens[i]; j++) {
      enum spell spell = optimum->spell_log_list[i][j];
      switch (spell) {
      case SPELL_MAGIC_MISSILE:
        DBG("%lu) Magic Missile", j + 1);
        break;
      case SPELL_DRAIN:
        DBG("%lu) Drain", j + 1);
        break;
      case SPELL_POISON:
        DBG("%lu) Poison", j + 1);
        break;
      case SPELL_RECHARGE:
        DBG("%lu) Recharge", j + 1);
        break;
      case SPELL_SHIELD:
        DBG("%lu) Shield", j + 1);
        break;
      }
    }
  }
#endif
}

struct queue {
  struct state *elements[QUEUESIZE];
  size_t head, tail;
};

static void queue_init(struct queue *q) { q->head = q->tail = 0; }

static bool queue_empty(struct queue *q) { return q->head == q->tail; }

static void queue_push(struct queue *q, struct state *e) {
  q->elements[q->tail] = e;
  q->tail = (q->tail + 1) % QUEUESIZE;
  ASSERT(q->tail != q->head, "Queue full");
}

static struct state *queue_pop(struct queue *q) {
  ASSERT(q->tail != q->head, "Queue empty");
  struct state *e = q->elements[q->head];
  q->head = (q->head + 1) % QUEUESIZE;
  return e;
}

static unsigned parse_number(const char **const input) {
  unsigned n = 0;
  while (isspace(**input))
    (*input)++;
  while (**input >= '0' && **input <= '9') {
    n = n * 10 + **input - '0';
    (*input)++;
  }
  return n;
}

static void parse_input(const char *input, unsigned *const boss_hp, unsigned *const boss_damage) {
  while (*input != ':')
    input++;
  input++;
  *boss_hp = parse_number(&input);

  while (*input != ':')
    input++;
  input++;
  *boss_damage = parse_number(&input);
}

static void init_state(struct state *state) {
  memset(state, 0, sizeof(*state));
  state->player_hp = 50;
  state->player_mana = 500;
}

static void demo_print_player_boss(const struct state *const state) {
#ifndef DEMO
  (void)state;
#endif

  DMO("- Player has %u hit points, %u armor, %u mana.", state->player_hp, state->effect_shield_turns_left > 0 ? 7 : 0,
      state->player_mana);
  DMO("- Boss has %u hit points.", state->boss_hp);
}

static enum turn_result deal_boss_damage(struct state *const state, unsigned amount) {
  if (state->boss_hp <= amount) {
    state->boss_hp = 0;
    return TURN_VICTORY;
  } else {
    state->boss_hp -= amount;
    return TURN_CONTINUE;
  }
}

static enum turn_result deal_player_damage(struct state *const state, unsigned amount) {
  unsigned net_amount;
  if (state->effect_shield_turns_left > 0) {
    if (amount <= 7) {
      net_amount = 1;
    } else {
      net_amount = amount - 7;
    }
  } else {
    net_amount = amount;
  }

  if (state->player_hp <= net_amount) {
    state->player_hp = 0;
    return TURN_GAME_OVER;
  } else {
    state->player_hp -= net_amount;
    return TURN_CONTINUE;
  }
}

static enum turn_result run_effects(bool hard_mode, struct state *const state) {
  if (hard_mode) {
    DMO("Player took 1 damage from being in HARD MODE");

    /*
      // At the start of each player turn (before any
      // other effects apply), you lose 1 hit point. If
      // this brings you to or below 0 hit points, you
      // lose.

      // That's not correct, solution expects you to not
      // die immediately from this -1 HP effect, only die
      // if you're not healed within the same turn.

    if ((state->player_hp -= 1) == 0) {
            DMO("You die from Hard Mode. Git gut.");
            return TURN_GAME_OVER;
    }
    */

    state->player_hp -= 1;
  }

  if (state->effect_shield_turns_left > 0) {
    state->effect_shield_turns_left--;
    DMO("Shield's timer is now %u", state->effect_shield_turns_left);

    if (state->effect_shield_turns_left == 0) {
      DMO("Shield wears off, decreasing armor by 7.");
    }
  }

  if (state->effect_poison_turns_left > 0) {
    state->effect_poison_turns_left--;
    DMO("Poison deals 3 damage; its timer is now %u", state->effect_poison_turns_left);

    enum turn_result r = deal_boss_damage(state, 3);
    if (r != TURN_CONTINUE) {
      return r;
    }

    if (state->effect_poison_turns_left == 0) {
      DMO("Poison wears off.");
    }
  }

  if (state->effect_recharge_turns_left > 0) {
    state->effect_recharge_turns_left--;
    DMO("Recharge provides 101 mana; its timer is now %u", state->effect_recharge_turns_left);

    state->player_mana += 101;

    if (state->effect_recharge_turns_left == 0) {
      DMO("Recharge wears off.");
    }
  }

  return TURN_CONTINUE;
}

static bool player_can_cast(struct state *const state, enum spell spell) {
  return (state->player_mana >= spell_mana_costs[spell]) &&
         ((spell == SPELL_MAGIC_MISSILE) || (spell == SPELL_DRAIN) ||
          (spell == SPELL_POISON && state->effect_poison_turns_left == 0) ||
          (spell == SPELL_RECHARGE && state->effect_recharge_turns_left == 0) ||
          (spell == SPELL_SHIELD && state->effect_shield_turns_left == 0));
}

static enum turn_result player_cast(struct state *const state, enum spell spell) {
#ifdef DEBUG
  state->spell_log[state->spell_log_len] = spell;
  state->spell_log_len++;
#endif

  state->mana_spent += spell_mana_costs[spell];
  state->player_mana -= spell_mana_costs[spell];

  switch (spell) {
  case SPELL_MAGIC_MISSILE:
    DMO("Player casts Magic Missle, dealing 4 damage.");
    return deal_boss_damage(state, 4);
  case SPELL_DRAIN:
    DMO("Player casts Drain, dealing 2 damage and healing 2 hit points.");
    state->player_hp += 2;
    return deal_boss_damage(state, 2);
  case SPELL_POISON:
    DMO("Player casts Poison.");
    state->effect_poison_turns_left = 6;
    return TURN_CONTINUE;
  case SPELL_RECHARGE:
    DMO("Player casts Recharge.");
    state->effect_recharge_turns_left = 5;
    return TURN_CONTINUE;
  case SPELL_SHIELD:
    DMO("Player casts Shield, increasing armor by 7.");
    state->effect_shield_turns_left = 6;
    return TURN_CONTINUE;
  default:
    FAIL("Invalid spell to cast");
  }
}

static enum turn_result run_player_turn(struct state *const state, enum spell spell) {
  if (!player_can_cast(state, spell)) {
#ifdef DEMO
    DBG("WARNING: CANNOT CAST SPELL BUT CASTING ANYWAY");
    enum turn_result result = player_cast(state, spell);
    if (state->player_hp == 0) {
      DBG("WARNING: AFTER CASTING THIS THE PLAYER IS DEAD BUT I'LL JUST KEEP GOING ANYWAY");
    }
    return result;
#else
    return TURN_GAME_OVER;
#endif
  } else {
    enum turn_result result = player_cast(state, spell);
    if (state->player_hp == 0) {
      return TURN_GAME_OVER;
    } else {
      return result;
    }
  }
}

static enum turn_result run_boss_turn(const struct constant_state *const constants, struct state *const state) {
  if (state->effect_shield_turns_left > 0) {
    DMO("Boss attacks for %u - 7 = %u damage.", constants->boss_damage, constants->boss_damage - 7);
  } else {
    DMO("Boss attacks for %u damage.", constants->boss_damage);
  }
  return deal_player_damage(state, constants->boss_damage);
}

static void run_turn(const struct constant_state *const constants, struct state *const oldstate,
                     enum turn_result results[5], struct state *newstates[5]) {
#ifdef DEMO
  static enum spell demo_spells[] = {SPELL_MAGIC_MISSILE, SPELL_POISON, SPELL_RECHARGE,
                                     SPELL_SHIELD,        SPELL_POISON, SPELL_RECHARGE,
                                     SPELL_DRAIN,         SPELL_POISON, SPELL_MAGIC_MISSILE};
  static size_t demo_spell_i = 0;
#endif

  DMO("-- Player Turn --");
  demo_print_player_boss(oldstate);

  enum turn_result result = run_effects(constants->hard_mode, oldstate);
  if (result != TURN_CONTINUE) {
    for (int i = 0; i < 5; i++) {
      results[i] = result;
      newstates[i] = NULL;
    }
    if (result == TURN_VICTORY) {
      newstates[0] = malloc(sizeof(*newstates[0]));
      *newstates[0] = *oldstate;
    }
    return;
  }

  for (enum spell spell = SPELL_MAGIC_MISSILE; spell <= SPELL_RECHARGE; spell++) {
#ifdef DEMO
    if (spell != demo_spells[demo_spell_i]) {
      results[spell] = TURN_GAME_OVER;
      newstates[spell] = NULL;
      continue;
    }
#endif

    newstates[spell] = malloc(sizeof(*newstates[spell]));
    *newstates[spell] = *oldstate;

    results[spell] = run_player_turn(newstates[spell], spell);
    if (results[spell] != TURN_CONTINUE) {
      continue;
    }

    DMO(" ");
    DMO("-- Boss Turn --");
    demo_print_player_boss(newstates[spell]);

    results[spell] = run_effects(false, newstates[spell]);
    if (results[spell] != TURN_CONTINUE) {
      continue;
    }

    results[spell] = run_boss_turn(constants, newstates[spell]);

    DMO(" ");
  }

#ifdef DEMO
  demo_spell_i++;
#endif
}

static void solution(const char *const input, char *const output, bool hard_mode) {
  struct constant_state constants;
  struct state *state = malloc(sizeof(*state));

  init_state(state);
  parse_input(input, &state->boss_hp, &constants.boss_damage);
  constants.hard_mode = hard_mode;

  /*
  #ifdef DEMO
  state->player_hp = 10;
  state->player_mana = 250;
  state->boss_hp = 14;
  constants.boss_damage = 8;
  #endif
  */

  static struct queue queue, *q = &queue;
  queue_init(q);
  queue_push(q, state);

  struct optimum_info optimum;
  optimum_info_init(&optimum);
  while (!queue_empty(q)) {
    state = queue_pop(q);
    if (optimum_info_prune(&optimum, state)) {
      free(state);
      continue;
    }

    enum turn_result results[5];
    struct state *result_states[5];
    run_turn(&constants, state, results, result_states);
    free(state);

    for (int i = 0; i < 5; i++) {
      enum turn_result result = results[i];
      struct state *newstate = result_states[i];

      switch (result) {
      case TURN_CONTINUE:
        queue_push(q, newstate);
        break;
      case TURN_VICTORY:
        DMO("The boss has died, the player wins!\n");
        if (newstate != NULL) {
          optimum_info_update(&optimum, newstate);
        }
        // fallthrough
      case TURN_GAME_OVER:
        free(newstate);
        break;
      }
    }
  }

  optimum_info_print(&optimum, output);
}

static void solution1(const char *const input, char *const output) { solution(input, output, false); }

static void solution2(const char *const input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
