#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define MAX_BOTS 256
#define MAX_INPUTS 32
#define MAX_OUTPUTS 32

enum highlow {
  HIGH,
  LOW,
};

struct bot {
  bool computed;

  enum highlow highlow1;
  enum highlow highlow2;
  struct entity *input1;
  struct entity *input2;

  unsigned high;
  unsigned low;
};

struct input {
  unsigned value;
};

struct entity {
  enum {
    ENTITY_BOT,
    ENTITY_INPUT,
  } type;
  union {
    struct bot bot;
    struct input input;
  } entity;
};

struct output {
  enum highlow highlow;
  struct entity *value;
};

struct entity bots[MAX_BOTS];
struct entity inputs[MAX_INPUTS];
struct output outputs[MAX_OUTPUTS];

static void init_buffers(void) {
  for (int i = 0; i < MAX_BOTS; i++) {
    bots[i].type = ENTITY_BOT;
    bots[i].entity.bot.computed = false;
    bots[i].entity.bot.input1 = NULL;
    bots[i].entity.bot.input2 = NULL;
  }
  for (int i = 0; i < MAX_INPUTS; i++) {
    inputs[i].type = ENTITY_INPUT;
  }
}

static void compute_bot(struct bot *const bot);
static unsigned get_bot_input(enum highlow highlow, struct entity *input) {
  switch (input->type) {
  case ENTITY_BOT:
    compute_bot(&input->entity.bot);
    switch (highlow) {
    case HIGH:
      return input->entity.bot.high;
    case LOW:
      return input->entity.bot.low;
    default:
      FAIL("unexpected high low");
    }
  case ENTITY_INPUT:
    return input->entity.input.value;
  default:
    FAIL("unexpected entity");
  }
}
static unsigned get_bot_input1(const struct bot *const bot) { return get_bot_input(bot->highlow1, bot->input1); }
static unsigned get_bot_input2(const struct bot *const bot) { return get_bot_input(bot->highlow2, bot->input2); }

static void compute_bot(struct bot *const bot) {
  if (bot->computed) {
    return;
  }

#ifdef DEBUG
  size_t idx = (((size_t)bot) - (offsetof(struct entity, entity)) - ((size_t)&bots)) / sizeof(*bot);
#endif

  DBG("compute bot %lu", idx);

  unsigned input1 = get_bot_input1(bot);
  DBG("(%lu) got input1=%u", idx, input1);
  unsigned input2 = get_bot_input2(bot);
  DBG("(%lu) got input2=%u", idx, input2);

  if (input1 > input2) {
    bot->high = input1;
    bot->low = input2;
  } else {
    bot->high = input2;
    bot->low = input1;
  }

  bot->computed = true;
}

static void assign_input_to_bot(struct bot *bot, struct entity *input, enum highlow highlow) {
  if (bot->input1 == NULL) {
    bot->input1 = input;
    bot->highlow1 = highlow;
  } else if (bot->input2 == NULL) {
    bot->input2 = input;
    bot->highlow2 = highlow;
  } else {
    FAIL("parse error");
  }
}

static void assign_input(bool to_output, unsigned idx, struct entity *input, enum highlow highlow) {
  if (to_output) {
    outputs[idx].value = input;
    outputs[idx].highlow = highlow;
  } else {
    assign_input_to_bot(&bots[idx].entity.bot, input, highlow);
  }
}

static void parse_advance(const char **const input, const char *const expect) {
  size_t len = strlen(expect);
  ASSERT(strncmp(*input, expect, len) == 0, "parse error");
  *input += len;
}

static unsigned parse_number(const char **const input) {
  unsigned n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static void parse_move_line(const char **const input, unsigned *const ibot, unsigned *const ilow, unsigned *const ihigh,
                            bool *const lowout, bool *const highout) {
  parse_advance(input, "bot ");
  *ibot = parse_number(input);
  parse_advance(input, " gives low to ");
  switch (**input) {
  case 'b':
    *lowout = false;
    parse_advance(input, "bot ");
    break;
  case 'o':
    *lowout = true;
    parse_advance(input, "output ");
    break;
  default:
    FAIL("parse error");
  }
  *ilow = parse_number(input);
  parse_advance(input, " and high to ");
  switch (**input) {
  case 'b':
    *highout = false;
    parse_advance(input, "bot ");
    break;
  case 'o':
    *highout = true;
    parse_advance(input, "output ");
    break;
  default:
    FAIL("parse error");
  }
  *ihigh = parse_number(input);
  parse_advance(input, "\n");
}

static void parse_assign_line(const char **const input, unsigned *const value, unsigned *const ibot) {
  parse_advance(input, "value ");
  *value = parse_number(input);
  parse_advance(input, " goes to bot ");
  *ibot = parse_number(input);
  parse_advance(input, "\n");
}

static void parse_line(const char **const input, size_t *nbots, size_t *incount) {
  switch (**input) {
  case 'b': {
    unsigned ibot, ilow, ihigh;
    bool lowout, highout;
    parse_move_line(input, &ibot, &ilow, &ihigh, &lowout, &highout);

    struct entity *botent = &bots[ibot];
    assign_input(lowout, ilow, botent, LOW);
    assign_input(highout, ihigh, botent, HIGH);

    if (ibot >= *nbots) {
      *nbots = ibot + 1;
    }
  } break;
  case 'v': {
    unsigned value, ibot;
    parse_assign_line(input, &value, &ibot);

    struct entity *inent = &inputs[(*incount)++];
    assign_input_to_bot(&bots[ibot].entity.bot, inent, LOW);
    inent->entity.input.value = value;

    if (ibot >= *nbots) {
      *nbots = ibot + 1;
    }
  } break;
  default:
    FAIL("unexpected input");
  }
}

static void parse(const char *input, size_t *const nbots) {
  size_t incount = 0;

  while (*input != '\0') {
    parse_line(&input, nbots, &incount);
    if (*input == '\n') {
      break;
    }
  }
}

static void solution1(const char *const input, char *const output) {
  init_buffers();

  size_t nbots = 0;
  parse(input, &nbots);

  size_t result = 0;
  for (size_t i = 0; i < nbots; i++) {
    compute_bot(&bots[i].entity.bot);
    if (bots[i].entity.bot.high == 61 && bots[i].entity.bot.low == 17) {
      result = i;
      break;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
}

static void solution2(const char *const input, char *const output) {
  init_buffers();

  size_t nbots = 0;
  parse(input, &nbots);

  size_t result = 1;
  for (int i = 0; i < 3; i++) {
    compute_bot(&outputs[i].value->entity.bot);
    switch (outputs[i].highlow) {
    case HIGH:
      result *= outputs[i].value->entity.bot.high;
      break;
    case LOW:
      result *= outputs[i].value->entity.bot.low;
      break;
    default:
      FAIL("unexpected high low tag");
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
