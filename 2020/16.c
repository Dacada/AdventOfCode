#define _POSIX_C_SOURCE 200809L // strndup

#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct range {
        int from, to;
};

struct rule {
        char *name;
        struct range *ranges;
        size_t ranges_len;
};

struct ticket {
        int *fields;
        size_t fields_len;
};

struct problem {
        struct rule *rules;
        size_t rules_len;
        struct ticket my_ticket;
        struct ticket *nearby_tickets;
        size_t nearby_tickets_len;
};

static void free_rule(const struct rule *const r) {
        free(r->name);
        free(r->ranges);
}

static void free_ticket(const struct ticket *const t) {
        free(t->fields);
}

static void free_problem(const struct problem *const p) {
        for (size_t i=0; i<p->rules_len; i++) {
                free_rule(p->rules+i);
        }
        free(p->rules);

        free_ticket(&p->my_ticket);
        for (size_t i=0; i<p->nearby_tickets_len; i++) {
                free_ticket(p->nearby_tickets+i);
        }
        free(p->nearby_tickets);
}

// parse int, expect input at first digit of integer, move it to character
// after last digit
static int parse_int(const char **const input) {
        int n = 0;
        char c;
        while (isdigit(c = **input)) {
                n = n*10+c-'0';
                *input += 1;
        }
        return n;
}

// parse a range, expect input to be at the first character of range, move it
// to the one after the last
static struct range parse_range(const char **const input) {
        struct range r;
        r.from = parse_int(input);
        ASSERT(**input == '-', "parse error %s", *input);
        *input += 1;
        r.to = parse_int(input);
        return r;
}

// return length of tag at *input and modify input to point at character right
// after colon
static size_t parse_tag(const char **const input) {
        char *colon = strchr(*input, ':');
        ASSERT(colon != NULL, "parse error");
        size_t ret = colon - *input;
        *input = colon+1;
        return ret;
}

// assert next input is given tag followed by the colon and an end of
// line, advance to the character right after the end of line
static void assert_tag(const char **const input, const char *const tag) {
        const char *t = *input;
        size_t taglen = parse_tag(input);
        ASSERT(strncmp(t, tag, taglen) == 0, "parse error");
        ASSERT(**input == '\n', "parse error");
        *input += 1;
}

// parse all " or " separated rule ranges, leaving input at the end of line
// character at the end
static struct range *parse_rule_ranges(const char **const input, size_t *const len) {
        size_t capacity = 2;
        struct range *list = malloc(sizeof(*list)*capacity);
        *len = 0;
        
        for (;;) {
                if (*len >= capacity) {
                        capacity *= 2;
                        list = realloc(list, sizeof(*list)*capacity);
                }
                
                list[*len] = parse_range(input);
                *len += 1;

                if (**input == ' ') {
                        ASSERT(strncmp(*input, " or ", 4) == 0, "parse error");
                        *input += 4;
                } else {
                        ASSERT(**input == '\n', "parse error");
                        break;
                }
        }
        
        return list;
}

// parse a rule, modify input to point at the \n at the end of the rule
static struct rule parse_rule(const char **const input) {
        struct rule r;
        
        const char *tag = *input;
        size_t taglen = parse_tag(input);
        r.name = strndup(tag, taglen);

        ASSERT(**input == ' ', "parse error");
        *input += 1;
        r.ranges = parse_rule_ranges(input, &r.ranges_len);

        return r;
}

// parse comma separated list of integers, modify input to point at the
// after the last integer
static int *parse_fields(const char **const input, size_t *const len) {
        size_t capacity = 8;
        int *list = malloc(sizeof(*list)*capacity);
        *len = 0;
        
        for (;;) {
                if (*len >= capacity) {
                        capacity *= 2;
                        list = realloc(list, sizeof(*list)*capacity);
                }
                
                list[*len] = parse_int(input);
                *len += 1;
                
                if (**input != ',') {
                        break;
                } else {
                        *input += 1;
                }
        }
        
        return list;
}

// parse a ticket, modify input to point at the \n at the end of a ticket
static struct ticket parse_ticket(const char **const input) {
        struct ticket t;
        t.fields = parse_fields(input, &t.fields_len);
        return t;
}

// parse my ticket block, modify input to point at the second \n at the end of
// the block
static struct ticket parse_my_ticket(const char **const input) {
        assert_tag(input, "your ticket");
        struct ticket result = parse_ticket(input);
        *input += 1;
        ASSERT(**input == '\n', "parse error");
        return result;
}

// parse all rules, modify input to point at the second \n at the end of rules
// block
static struct rule *parse_rules(const char **const input, size_t *const len) {
        size_t capacity = 16;
        struct rule *list = malloc(sizeof(*list)*capacity);
        *len = 0;
        
        for (;**input!='\n'; *input+=1) {
                if (*len >= capacity) {
                        capacity *= 2;
                        list = realloc(list, sizeof(*list)*capacity);
                }
                
                list[*len] = parse_rule(input);
                *len += 1;
        }
        
        return list;
}

// parse nearby tickets block, modify input to point at the \0 at the end of
// the block and end of input
static struct ticket *parse_nearby_tickets(const char **const input, size_t *const len) {
        assert_tag(input, "nearby tickets");
        
        size_t capacity = 32;
        struct ticket *list = malloc(sizeof(*list)*capacity);
        *len = 0;
        
        for (;**input!='\0' ;*input+=1) {
                if (*len >= capacity) {
                        capacity *= 2;
                        list = realloc(list, sizeof(*list)*capacity);
                }
                
                list[*len] = parse_ticket(input);
                *len += 1;
        }
        
        return list;
}

static struct problem parse_problem(const char *input) {
        struct problem p;
        
        p.rules = parse_rules(&input, &p.rules_len);
        ASSERT(*input == '\n', "parse error");
        input++;
        
        p.my_ticket = parse_my_ticket(&input);
        ASSERT(*input == '\n', "parse error");
        input++;
        
        p.nearby_tickets = parse_nearby_tickets(&input, &p.nearby_tickets_len);
        ASSERT(*input == '\0', "parse error");
        
        return p;
}

static bool follows_rule(const int field, const struct rule *const rule) {
        for (size_t l=0; l<rule->ranges_len; l++) {
                struct range *range = rule->ranges+l;
                int from = range->from;
                int to = range->to;
                if (from <= field && to >= field) {
                        return true;
                }
        }
        return false;
}

static long filter_fields(const struct problem *const problem, bool *const valid_ticket) {
        long sum = 0;
        
        for (size_t i=0; i<problem->nearby_tickets_len; i++) {
                struct ticket *ticket = problem->nearby_tickets+i;
                bool is_valid = true;
                for (size_t j=0; j<ticket->fields_len; j++) {
                        int field = ticket->fields[j];
                        bool valid = false;
                        for (size_t k=0; k<problem->rules_len; k++) {
                                struct rule *rule = problem->rules+k;
                                valid |= follows_rule(field, rule);
                                if (valid) {
                                        break;
                                }
                        }
                        if (!valid) {
                                sum += field;
                                is_valid = false;
                        }
                }

                if (is_valid && valid_ticket != NULL) {
                        valid_ticket[i] = true;
                }
        }
        
        return sum;
}

static void solution1(const char *const input, char *const output) {
        const struct problem problem = parse_problem(input);
        long sum = filter_fields(&problem, NULL);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", sum);
        free_problem(&problem);
}

static void solution2(const char *const input, char *const output) {
        const struct problem problem = parse_problem(input);
        
        const size_t ntickets = problem.nearby_tickets_len;
        const size_t nfields = problem.nearby_tickets[0].fields_len;
        const size_t nrules = problem.rules_len;
        
        // candidates: the ith rule is a candidate for the jth field if
        // candidates[j*nrules + i] is true
        bool *candidates = malloc(sizeof(*candidates)*nfields*nrules);

        // valid_ticket: the ith ticket is valid if valid_ticket[i] is true
        bool *valid_ticket = malloc(sizeof(*valid_ticket)*ntickets);
	memset(valid_ticket, 0, sizeof(*valid_ticket)*ntickets);
        
        filter_fields(&problem, valid_ticket);

        // All rules start as a candidate for every field
        for (size_t i=0; i<nfields*nrules; i++) {
                candidates[i] = true;
        }

        for (size_t i=0; i<ntickets; i++) {
                struct ticket *ticket = problem.nearby_tickets+i;
                if (valid_ticket[i]) {
                        DBG("Ticket %lu is valid.", i);
                        for (size_t j=0; j<ticket->fields_len; j++) {
                                int field = ticket->fields[j];
                                for (size_t k=0; k<problem.rules_len; k++) {
                                        struct rule *rule = problem.rules+k;
                                        if (!follows_rule(field, rule)) {
                                                DBG("Field %lu does not follow rule %lu.", j, k);
                                                candidates[j*nrules + k] = false;
                                        } else {
                                                DBG("Field %lu follows rule %lu.", j, k);
                                        }
                                }
                        }
                }
        }

        // Now search for fields that are candidate to only one rule. These are
        // confirmed, and so all other fields should no longer be candidates
        // for that rule, so we remove them. Keep doing this until all rules
        // have only one candidate field.

        // keep track of which ones we've decided on to avoid doing the removal
        // of candidates over and over for the same field
        bool *decided_fields = malloc(sizeof(*decided_fields)*nfields);
        memset(decided_fields, 0, sizeof(*decided_fields)*nrules);
        
        bool done = false; // done = all(decided_fields)
        while (!done) {
                done = true;
                for (size_t j=0; j<nfields; j++) {
                        size_t candidate_count = 0;
                        size_t which_rule = 0;
                        for (size_t i=0; i<nrules; i++) {
                                if (candidates[j*nrules+i]) {
                                        candidate_count++;
                                        which_rule = i;
                                }
                        }
                        
                        ASSERT(candidate_count > 0, "ran out of candidates?");
                        if (candidate_count == 1) {
                                DBG("Field %lu has only one candidate rule (%lu). "
                                    "It's removed from all other fields.", j, which_rule);
                                if (!decided_fields[j]) {
                                        decided_fields[j] = true;
                                        for (size_t jj=0; jj<nfields; jj++) {
                                                if (j == jj) {
                                                        continue;
                                                }
                                                candidates[jj*nrules + which_rule] = false;
                                        }
                                }
                        } else {
                                done = false;
                        }
                }
        }

        long mul = 1;
        for (size_t i=0; i<nrules; i++) {
                struct rule *rule = problem.rules+i;
                for (size_t j=0; j<nfields; j++) {
                        if (candidates[j*nrules + i]) {
                                if (strncmp(rule->name, "departure", strlen("departure")) == 0) {
                                        mul *= problem.my_ticket.fields[j];
                                }
                                DBG("Field %lu is %s", j, rule->name);
                        }
                }
        }

        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", mul);
        free_problem(&problem);
        free(valid_ticket);
        free(candidates);
        free(decided_fields);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
