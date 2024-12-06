#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define ID_ACCEPT -1
#define ID_REJECT -2

enum operation {
  LESSER_THAN,
  GREATER_THAN,
};

enum trait {
  EXTREME,
  MUSICAL,
  AERODYNAMIC,
  SHINY,
};

struct part {
  int x;
  int m;
  int a;
  int s;
};

struct rule {
  enum operation op;
  enum trait trait;
  int value;
  int next_id;
};

struct workflow {
  int id;
  int nrules;
  struct rule *rules;
  int dflt_id;
};

static void workflow_free(struct workflow *w) { free(w->rules); }

static void assert_string(const char **str, const char *expected) {
  int n = strlen(expected);
  ASSERT(strncmp(*str, expected, n) == 0, "parse error '%s' '%s'", *str, expected);
  *str += n;
}

static int parse_int(const char **input) {
  ASSERT(isdigit(**input), "parse error");
  int n = 0;
  while (isdigit(**input)) {
    n *= 10;
    n += **input - '0';
    *input += 1;
  }
  return n;
}

static int parse_id(const char **input) {
  if (**input == 'A') {
    *input += 1;
    return ID_ACCEPT;
  } else if (**input == 'R') {
    *input += 1;
    return ID_REJECT;
  }

  ASSERT(islower(**input), "parse error");
  int n = 0;
  while (islower(**input)) {
    n *= 'z' - 'a' + 1;
    n += **input - 'a';
    *input += 1;
  }
  return n;
}

#ifdef DEBUG
static const char *id_to_str(int id) {
  static char str[4];
  memset(str, 0, sizeof(str));

  if (id == ID_ACCEPT) {
    return "A";
  } else if (id == ID_REJECT) {
    return "R";
  }

  int i;
  for (i = 0; i < 3; i++) {
    str[2 - i] = id % 26 + 'a';
    id /= 26;
    if (id == 0) {
      break;
    }
  }
  ASSERT(id == 0, "id too long");

  return &str[2 - i];
}
#endif

static bool parse_rule(const char **input, struct rule *rule) {
  enum trait t;
  switch (**input) {
  case 'x':
    t = EXTREME;
    break;
  case 'm':
    t = MUSICAL;
    break;
  case 'a':
    t = AERODYNAMIC;
    break;
  case 's':
    t = SHINY;
    break;
  default:
    return false;
  }

  enum operation op;
  switch (*(*input + 1)) {
  case '<':
    op = LESSER_THAN;
    break;
  case '>':
    op = GREATER_THAN;
    break;
  default:
    return false;
  }

  *input += 2;

  int v = parse_int(input);

  ASSERT(**input == ':', "parse error");
  *input += 1;

  int id = parse_id(input);

  rule->op = op;
  rule->trait = t;
  rule->value = v;
  rule->next_id = id;
  return true;
}

static struct rule *parse_rules(const char **input, int *nrules) {
  int len = 0;
  int cap = 1;
  struct rule *list = malloc(sizeof(*list) * cap);

  for (;;) {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    if (!parse_rule(input, &list[len])) {
      break;
    }
    len++;

    ASSERT(**input == ',', "parse error");
    *input += 1;
  }

  *nrules = len;
  return list;
}

static void parse_workflow(const char **input, struct workflow *workflow) {
  workflow->id = parse_id(input);

  ASSERT(**input == '{', "parse error");
  *input += 1;

  workflow->rules = parse_rules(input, &workflow->nrules);
  workflow->dflt_id = parse_id(input);

  assert_string(input, "}\n");
}

static struct workflow *parse_workflows(const char **input, int *nworkflows) {
  int len = 0;
  int cap = 8;
  struct workflow *list = malloc(sizeof(*list) * cap);

  while (**input != '\n') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    parse_workflow(input, &list[len++]);
  }

  *nworkflows = len;
  return list;
}

static void parse_part(const char **input, struct part *part) {
  assert_string(input, "{x=");
  part->x = parse_int(input);

  assert_string(input, ",m=");
  part->m = parse_int(input);

  assert_string(input, ",a=");
  part->a = parse_int(input);

  assert_string(input, ",s=");
  part->s = parse_int(input);

  ASSERT(**input == '}', "parse error");
  *input += 1;

  if (**input == '\n') {
    *input += 1;
  }
}

static struct part *parse_parts(const char **input, int *nparts) {
  int len = 0;
  int cap = 4;
  struct part *list = malloc(sizeof(*list) * cap);

  while (**input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    parse_part(input, &list[len++]);
  }

  *nparts = len;
  return list;
}

static void parse_input(const char *input, struct workflow **workflows, int *nworkflows, struct part **parts,
                        int *nparts) {
  *workflows = parse_workflows(&input, nworkflows);
  input += 1;
  *parts = parse_parts(&input, nparts);
}

static bool rule_matches(struct rule *rule, struct part *part) {
  int part_value = *(&part->x + rule->trait);
  if (part_value < rule->value) {
    return rule->op == LESSER_THAN;
  } else if (part_value > rule->value) {
    return rule->op == GREATER_THAN;
  } else {
    return false;
  }
}

static int get_next_workflow_id(struct workflow *workflow, struct part *part) {
  for (int i = 0; i < workflow->nrules; i++) {
    if (rule_matches(&workflow->rules[i], part)) {
      return workflow->rules[i].next_id;
    }
  }
  return workflow->dflt_id;
}

static bool process_part(struct workflow *workflow, struct part *part, struct workflow **index) {
#ifdef DEBUG
  fprintf(stderr, "%s -> ", id_to_str(workflow->id));
#endif

  int id = get_next_workflow_id(workflow, part);
  if (id == ID_ACCEPT) {
#ifdef DEBUG
    fprintf(stderr, "A\n");
#endif
    return true;
  } else if (id == ID_REJECT) {
#ifdef DEBUG
    fprintf(stderr, "R\n");
#endif
    return false;
  } else {
    return process_part(index[id], part, index);
  }
}

static void solution1(const char *const input, char *const output) {
  struct workflow *workflows;
  struct part *parts;
  int nworkflows, nparts;
  parse_input(input, &workflows, &nworkflows, &parts, &nparts);

#ifdef DEBUG
  for (int i = 0; i < nworkflows; i++) {
    struct workflow *w = &workflows[i];
    fprintf(stderr, "%s{", id_to_str(w->id));
    for (int j = 0; j < w->nrules; j++) {
      struct rule *r = &w->rules[j];
      fprintf(stderr, "%c%c%d:%s,", "xmas"[r->trait], "<>"[r->op], r -> value, id_to_str(r->next_id));
    }
    fprintf(stderr, "%s}\n", id_to_str(w->dflt_id));
  }
  fputc('\n', stderr);
  for (int i = 0; i < nparts; i++) {
    struct part *p = &parts[i];
    fprintf(stderr, "{x=%d,m=%d,a=%d,s=%d}\n", p->x, p->m, p->a, p->s);
  }
#endif

  static struct workflow *index[26 * 26 * 26];
  memset(index, 0, sizeof(index));
  for (int i = 0; i < nworkflows; i++) {
    struct workflow *workflow = &workflows[i];
    index[workflow->id] = workflow;
  }

  int res = 0;
  for (int i = 0; i < nparts; i++) {
    const char *in = "in";
    if (process_part(index[parse_id(&in)], &parts[i], index)) {
      res += parts[i].x;
      res += parts[i].m;
      res += parts[i].a;
      res += parts[i].s;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
  for (int i = 0; i < nworkflows; i++) {
    workflow_free(&workflows[i]);
  }
  free(workflows);
  free(parts);
}

// like a python range, start is inclusive and stop isn't
struct segment {
  long start;
  long stop;
};

static long segment_len(struct segment s) { return s.stop - s.start; }

struct possible_part {
  struct segment x;
  struct segment m;
  struct segment a;
  struct segment s;
};

static long possible_part_size(struct possible_part p) {
  long res = 1;
  for (int i = 0; i < 4; i++) {
    res *= segment_len(*(&p.x + i));
  }
  return res;
}

static void apply_rule_to_possible_part(struct rule *rule, struct possible_part *obeys,
                                        struct possible_part *disobeys) {
  struct segment *segment_obeys = &obeys->x + rule->trait;
  struct segment *segment_disobeys = &disobeys->x + rule->trait;

  switch (rule->op) {
  case LESSER_THAN:
    segment_obeys->stop = rule->value;
    segment_disobeys->start = rule->value;
    break;
  case GREATER_THAN:
    segment_obeys->start = rule->value + 1;
    segment_disobeys->stop = rule->value + 1;
    break;
  default:
    FAIL("bad value");
  }
}

static long accepted_possible_part_size(struct workflow *workflow, struct possible_part part, struct workflow **index);
__attribute__((pure)) static long accepted_possible_part_size_from_id(int id, struct possible_part part,
                                                                      struct workflow **index) {
  if (id == ID_ACCEPT) {
    return possible_part_size(part);
  } else if (id == ID_REJECT) {
    return 0;
  } else {
    return accepted_possible_part_size(index[id], part, index);
  }
}
static long accepted_possible_part_size(struct workflow *workflow, struct possible_part part, struct workflow **index) {
  struct possible_part current = part;

  long res = 0;
  for (int i = 0; i < workflow->nrules; i++) {
    struct rule *rule = &workflow->rules[i];
    struct possible_part this_part = current;
    apply_rule_to_possible_part(rule, &this_part, &current);
    res += accepted_possible_part_size_from_id(rule->next_id, this_part, index);
  }

  res += accepted_possible_part_size_from_id(workflow->dflt_id, current, index);
  return res;
}

static void solution2(const char *const input, char *const output) {
  struct workflow *workflows;
  struct part *parts;
  int nworkflows, nparts;
  parse_input(input, &workflows, &nworkflows, &parts, &nparts);
  free(parts);

  static struct workflow *index[26 * 26 * 26];
  memset(index, 0, sizeof(index));
  for (int i = 0; i < nworkflows; i++) {
    struct workflow *workflow = &workflows[i];
    index[workflow->id] = workflow;
  }

  struct possible_part part;
  for (int i = 0; i < 4; i++) {
    (&part.x + i)->start = 1;
    (&part.x + i)->stop = 4001;
  }
  const char *in = "in";
  long res = accepted_possible_part_size_from_id(parse_id(&in), part, index);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
  for (int i = 0; i < nworkflows; i++) {
    workflow_free(&workflows[i]);
  }
  free(workflows);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
