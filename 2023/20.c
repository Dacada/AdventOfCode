#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//#define TRACE
#define QUEUE_MAX_SIZE (26*26)

enum module_type {
	MODULE_FLIPFLOP,
	MODULE_CONJUNCTION,
};

struct module {
	int id;
	enum module_type type;
	int noutputs;
	int *outputs;
};

static void module_init(struct module *module, enum module_type type) {
	module->id = 0;
	module->type = type;
	module->noutputs = 0;
	module->outputs = NULL;
}

struct module_flipflop {
	struct module base;
	bool state;
};

static void module_flipflop_init(struct module_flipflop *module) {
	module_init(&module->base, MODULE_FLIPFLOP);
	module->state = false;
}

struct module_conjunction {
	struct module base;
	int ninputs;
	int *input_ids;
	bool *input_states;
};

static void module_conjunction_init(struct module_conjunction *module) {
	module_init(&module->base, MODULE_CONJUNCTION);
	module->ninputs = 0;
	module->input_ids = NULL;
	module->input_states = NULL;
}

static void module_conjunction_free(struct module_conjunction *module) {
	free(module->input_ids);
	free(module->input_states);
}

static void module_free(struct module *module) {
	if (module == NULL) {
		return;
	}
	if (module->type == MODULE_CONJUNCTION) {
		module_conjunction_free((struct module_conjunction*)module);
	}
	free(module->outputs);
}

struct module_collection {
	struct module **modules;
	int nconnections_broadcaster;
	int *connections_broadcaster;
};

static void module_collection_init(struct module_collection *collection) {
	size_t s = sizeof(*collection->modules)*26*26;
	collection->modules = malloc(s);
	memset(collection->modules, 0, s);
	
	collection->nconnections_broadcaster = 0;
	collection->connections_broadcaster = NULL;
}

static void module_collection_free(struct module_collection *collection) {
	for (int i=0; i<26*26; i++) {
		module_free(collection->modules[i]);
		free(collection->modules[i]);
	}
	free(collection->modules);
	free(collection->connections_broadcaster);
}

static void module_collection_add_broadcaster_connections(struct module_collection *collection, int *connections, int nconnections) {
	collection->nconnections_broadcaster = nconnections;
	collection->connections_broadcaster = connections;
}

static void module_collection_add_module(struct module_collection *collection, struct module *module, int *connections, int nconnections) {
	module->outputs = connections;
	module->noutputs = nconnections;
	collection->modules[module->id] = module;
}

static void module_collection_init_conjunction_modules(struct module_collection *collection) {
	for (int i=-1; i<26*26; i++) {
		int noutputs;
		int *outputs;
		if (i == -1) {
			noutputs = collection->nconnections_broadcaster;
			outputs = collection->connections_broadcaster;
		} else {
			struct module *module = collection->modules[i];
			if (module == NULL) {
				continue;
			}
			noutputs = module->noutputs;
			outputs = module->outputs;
		}
		
		for (int j=0; j<noutputs; j++) {
			struct module *output = collection->modules[outputs[j]];
			if (output != NULL && output->type == MODULE_CONJUNCTION) {
				struct module_conjunction *o = (struct module_conjunction*)output;
				o->ninputs++;
				o->input_ids = realloc(o->input_ids, sizeof(o->input_ids)*o->ninputs);
				o->input_ids[o->ninputs-1] = i;
			}
		}
	}

	for (int i=0; i<26*26; i++) {
		struct module *module = collection->modules[i];
		if (module == NULL || module->type != MODULE_CONJUNCTION) {
			continue;
		}
		struct module_conjunction *m = (struct module_conjunction*)module;
		size_t s = sizeof(*m->input_states)*m->ninputs;
		m->input_states = malloc(s);
		memset(m->input_states, 0, s);
		ASSERT(m->input_ids != NULL, "module without inputs!");
	}
}

static void assert_string(const char **str, const char *expected) {
	int n = strlen(expected);
	ASSERT(strncmp(*str, expected, n) == 0, "parse error '%s' '%s'", *str, expected);
	*str += n;
}

static int parse_id(const char **input) {
	int n;
	
	ASSERT(islower(**input), "parse error");
	n = **input - 'a';
	*input += 1;
	n *= 26;

	ASSERT(islower(**input), "parse error");
	n += **input - 'a';
	*input += 1;

	return n;
}

static struct module *parse_origin_module(const char **input) {
	struct module *res;
	
	char c = **input;
	*input += 1;

	if (c == '%') {
		res = malloc(sizeof(struct module_flipflop));
		module_flipflop_init((struct module_flipflop*)res);
	} else if (c == '&') {
		res = malloc(sizeof(struct module_conjunction));
		module_conjunction_init((struct module_conjunction*)res);
	} else {
		FAIL("parse error %c", c);
	}

	res->id = parse_id(input);

	return res;
}

static int *parse_connection_list(const char **input, int *length) {
	int len = 0;
	int cap = 8;
	int *list = malloc(sizeof(*list)*cap);

	for (;;) {
		if (len >= cap) {
			cap *= 2;
			list = realloc(list, sizeof(*list)*cap);
		}

		list[len++] = parse_id(input);
		if (**input == '\n' || **input == '\0') {
			break;
		}
		assert_string(input, ", ");
	}

	*length = len;
	return list;
}

static void parse_line(const char **input, struct module_collection *modules) {
	struct module *module;
	if (strncmp(*input, "broadcaster", strlen("broadcaster")) == 0) {
		module = NULL;
		*input += strlen("broadcaster");
	} else {
		module = parse_origin_module(input);
	}

	assert_string(input, " -> ");

	int nconnections;
	int *connections;
	connections = parse_connection_list(input, &nconnections);

	if (module == NULL) {
		module_collection_add_broadcaster_connections(modules, connections, nconnections);
	} else {
		module_collection_add_module(modules, module, connections, nconnections);
	}

	if (**input == '\n') {
		*input += 1;
	}
}

static struct module_collection parse_input(const char *input) {
	struct module_collection modules;
	module_collection_init(&modules);

	while (*input != '\0') {
		parse_line(&input, &modules);
	}
	module_collection_init_conjunction_modules(&modules);

	return modules;
}

static void print_state(struct module_collection *modules) {
#ifdef DEBUG
	for (int i=-1; i<26*26; i++) {
		int noutputs;
		int *outputs;
		struct module *module;
		if (i == -1) {
			noutputs = modules->nconnections_broadcaster;
			outputs = modules->connections_broadcaster;
			module = NULL;
		} else {
			module = modules->modules[i];
			if (module == NULL) {
				continue;
			}
			noutputs = module->noutputs;
			outputs = module->outputs;
		}
		
		fprintf(stderr, "module '%c%c' outputs to ", i/26+'a', i%26+'a');
		for (int j=0; j<noutputs; j++) {
			fprintf(stderr, "%c%c", outputs[j]/26+'a', outputs[j]%26+'a');
			if (j < noutputs-1) {
				fputs(", ", stderr);
			}
		}
		fputc('\n', stderr);
		if (module == NULL) {
			fputs("  it is the broadcaster\n", stderr);
		} else if (module->type == MODULE_FLIPFLOP) {
			fputs("  it is a flipflop\n", stderr);
			fprintf(stderr, "  state=%d\n", ((struct module_flipflop*)module)->state);
		} else if (module->type == MODULE_CONJUNCTION) {
			struct module_conjunction *m = (struct module_conjunction*)module;
			fputs("  it is a conjunction\n", stderr);
			fputs("  inputs: ", stderr);
			for (int j=0; j<m->ninputs; j++) {
				fprintf(stderr, "%c%c(%d)", m->input_ids[j]/26+'a', m->input_ids[j]%26+'a', m->input_states[j]);
				if (j < m->ninputs-1) {
					fputs(", ", stderr);
				}
			}
			fputc('\n', stderr);
		} else {
			FAIL("module with invalid type!");
		}
	}
#else
	(void)modules;
#endif
}

static bool module_flipflop_recv(struct module_flipflop *module, bool pulse, bool *response) {
	if (pulse) {
		return false;
	}

	*response = module->state = !module->state;
	return true;
}

static bool module_conjunction_recv(struct module_conjunction *module, int sender_id, bool pulse, bool *response) {
	int i;
	bool found = false;
	for (i=0; i<module->ninputs; i++) {
		if (module->input_ids[i] == sender_id) {
			found = true;
			break;
		}
	}
	if (!found) {
		FAIL("input missmatch");
	}

	module->input_states[i] = pulse;

	bool all = true;
	for (i=0; i<module->ninputs; i++) {
		if (!module->input_states[i]) {
			all = false;
			break;
		}
	}

	*response = !all;
	return true;
}

static bool module_recv(struct module *module, int sender_id, bool pulse, bool *response) {
	switch (module->type) {
	case MODULE_FLIPFLOP:
		return module_flipflop_recv((struct module_flipflop*)module, pulse, response);
	case MODULE_CONJUNCTION:
		return module_conjunction_recv((struct module_conjunction*)module, sender_id, pulse, response);
	default: FAIL("bad type");
	}
}

struct signal {
	bool value;
	int from;
	struct module *to;
};

struct queue {
	int front;
	int tail;
	struct signal elements[QUEUE_MAX_SIZE];
};

static void queue_init(struct queue *q) {
	q->front = 0;
	q->tail = 0;
}

static void queue_push(struct queue *q, struct signal s) {
	q->elements[q->tail] = s;
	q->tail++;
	q->tail %= QUEUE_MAX_SIZE;
	ASSERT(q->tail != q->front, "queue full");
}

static struct signal queue_pop(struct queue *q) {
	ASSERT(q->tail != q->front, "queue empty");
	struct signal s = q->elements[q->front];
	q->front++;
	q->front %= QUEUE_MAX_SIZE;
	return s;
}

static bool queue_empty(const struct queue *q) {
	return q->front == q->tail;
}

static void push_button(struct module_collection *modules, int *highs, int *lows, int track_id, bool *had_high_input) {
#ifdef TRACE
	fprintf(stderr, "button -low-> broadcaster\n");
#endif
	if (lows != NULL) {
		*lows += 1;
	}

	static struct queue q;
	queue_init(&q);
	for (int i=0; i<modules->nconnections_broadcaster; i++) {
		struct module *module = modules->modules[modules->connections_broadcaster[i]];
		struct signal s = {.value=false, .from=-1, .to=module};
		queue_push(&q, s);
	}

	while (!queue_empty(&q)) {
		struct signal s = queue_pop(&q);

		if (s.value) {
			if (highs != NULL) {
				*highs += 1;
			}
		} else {
			if (lows != NULL) {
				*lows += 1;
			}
		}

#ifdef TRACE
		{
			const char *from;
			const char *to;
			char afrom[3];
			char ato[3];
			if (s.from == -1) {
				from = "broadcaster";
			} else {
				afrom[0] = s.from / 26 + 'a';
				afrom[1] = s.from % 26 + 'a';
				afrom[2] = '\0';
				from = afrom;
			}
			if (s.to == NULL) {
				to = "output";
			} else {
				ato[0] = s.to->id / 26 + 'a';
				ato[1] = s.to->id % 26 + 'a';
				ato[2] = '\0';
				to = ato;
			}
			fprintf(stderr, "%s -%s-> %s\n", from, s.value?"high":"low", to);
		}
#endif
		
		if (s.to == NULL) {
			// output module
			continue;
		}

		bool response;
		if (module_recv(s.to, s.from, s.value, &response)) {
			for (int i=0; i<s.to->noutputs; i++) {
				int next_id = s.to->outputs[i];
				struct module *next_module = modules->modules[next_id];
				struct signal new = {.value=response, .from=s.to->id, .to=next_module};
				if (had_high_input != NULL && new.to != NULL && new.to->id == track_id && new.value) {
					ASSERT(new.to->type == MODULE_CONJUNCTION, "track must be conjunction");
					*had_high_input = true;
				}
				queue_push(&q, new);
			}
		}
	}
}

static void solution1(const char *const input, char *const output) {
	struct module_collection modules = parse_input(input);
	print_state(&modules);

	int highs = 0;
	int lows = 0;
	for (int i=0; i<1000; i++) {
		push_button(&modules, &highs, &lows, 0, NULL);
	}
	
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", highs*lows);
	module_collection_free(&modules);
}

__attribute__((const))
static long gcd(long a, long b) {
	while (a != b) {
		if (a > b) {
			a -= b;
		} else {
			b -= a;
		}
	}
	return a;
}

__attribute__((const))
static long lcm(long a, long b) {
	return (a * b) / gcd(a, b);
}

static void solution2(const char *const input, char *const output) {
	struct module_collection modules = parse_input(input);

	int rx = ('r'-'a')*26 + 'x'-'a';

	// find the one node that outputs to rx
	int parent_id = -1;
	for (int i=0; i<26*26; i++) {
		struct module *m = modules.modules[i];
		if (m == NULL) {
			continue;
		}

		bool found = false;
		for (int j=0; j<m->noutputs; j++) {
			if (m->outputs[j] == rx) {
				found = true;
			}
		}
		if (found) {
			if (parent_id != -1) {
				FAIL("more than one parent for output?");
			}
			parent_id = i;
		}
	}
	ASSERT(parent_id != -1, "no parents for output?");
	struct module *p = modules.modules[parent_id];
	ASSERT(p->type == MODULE_CONJUNCTION, "parent should be conjunction");
	struct module_conjunction *parent = (struct module_conjunction *)p;

	long res = 1;
	int seen = 0;
	for (int i=0;; i++) {
		bool had_high_input = false;
		push_button(&modules, NULL, NULL, p->id, &had_high_input);
		if (had_high_input) {
			DBG("%d", i+1);
			res = lcm(res, i+1);
			seen++;
		}
		if (seen >= parent->ninputs) {
			break;
		}
	}

        snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", res);
	module_collection_free(&modules);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
