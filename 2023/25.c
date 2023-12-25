#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define QUEUE_MAX_SIZE (1<<12)

struct name {
	char name[3];
};

static int namecmp(const void *p1, const void *p2) {
	return strncmp(p1, p2, 3);
}

static void assert_string(const char **str, const char *expected) {
	int n = strlen(expected);
	ASSERT(strncmp(*str, expected, n) == 0, "parse error '%s' '%s'", *str, expected);
	*str += n;
}

static struct name parse_name(const char **input) {
	struct name name;
	for (int i=0; i<3; i++) {
		ASSERT(islower(**input), "parse error %s", *input);
		name.name[i] = **input;
		*input += 1;
	}
	return name;
}

static void remove_duplicates(struct name *list, int *length) {
	int j = 1;
	struct name prev = list[0];
	for (int i=1; i<*length; i++) {
		struct name curr = list[i];
		if (namecmp(&prev, &curr) == 0) {
			continue;
		} else {
			list[j++] = curr;
			prev = curr;
		}
	}
	*length = j;
}

static int name_idx(struct name *list, int len, struct name name) {
	struct name *ptr = bsearch(&name, list, len, sizeof(*list), namecmp);
	ASSERT(ptr != NULL, "logic error");
	int ret = ptr - list;
	ASSERT(ret < len, "logic error");
	return ret;
}

struct neighbor {
	int node;
	bool valid;
};

struct neighbor_list {
	struct neighbor *neighbors;
	int len;
};

static void neighbor_list_init(struct neighbor_list *list, int nnodes) {
	list->len = 0;
	list->neighbors = malloc(sizeof(*list->neighbors)*nnodes);
}

static void neighbor_list_free(struct neighbor_list *list) {
	free(list->neighbors);
}

static void neighbor_list_add(struct neighbor_list *list, int node) {
	list->neighbors[list->len].node = node;
	list->neighbors[list->len].valid = true;
	list->len++;
}

static int neighbor_list_ith(struct neighbor_list *list, int i) {
	struct neighbor neighbor = list->neighbors[i];
	if (neighbor.valid) {
		return neighbor.node;
	} else {
		return -1;
	}
}

static void neighbor_list_invalidate(struct neighbor_list *list, int node) {
	for (int i=0; i<list->len; i++) {
		if (list->neighbors[i].node == node) {
			list->neighbors[i].valid = false;
			break;
		}
	}
}

static void neighbor_list_revalidate(struct neighbor_list *list, int node) {
	for (int i=0; i<list->len; i++) {
		if (list->neighbors[i].node == node) {
			list->neighbors[i].valid = true;
			break;
		}
	}
}

struct adjacency_list {
	struct neighbor_list *nodes;
	int len;
};

static void adjacency_list_init(struct adjacency_list *list, int nnodes) {
	list->len = nnodes;
	list->nodes = malloc(sizeof(*list->nodes)*nnodes);
	for (int i=0; i<nnodes; i++) {
		neighbor_list_init(&list->nodes[i], nnodes);
	}
}

static void adjacency_list_free(struct adjacency_list *list) {
	for (int i=0; i<list->len; i++) {
		neighbor_list_free(&list->nodes[i]);
	}
	free(list->nodes);
}

static void adjacency_list_add(struct adjacency_list *list, int n1, int n2) {
	neighbor_list_add(&list->nodes[n1], n2);
	neighbor_list_add(&list->nodes[n2], n1);
}

static int adjacency_list_count_neighbors(const struct adjacency_list *list, int n) {
	return list->nodes[n].len;
}

static int adjacency_list_ith_neighbor(const struct adjacency_list *list, int n, int i) {
	return neighbor_list_ith(&list->nodes[n], i);
}

static void adjacency_list_invalidate_neighbors(struct adjacency_list *list, int n1, int n2) {
	neighbor_list_invalidate(&list->nodes[n1], n2);
	neighbor_list_invalidate(&list->nodes[n2], n1);
}

static void adjacency_list_revalidate_neighbors(struct adjacency_list *list, int n1, int n2) {
	neighbor_list_revalidate(&list->nodes[n1], n2);
	neighbor_list_revalidate(&list->nodes[n2], n1);
}

static struct adjacency_list parse_input(const char *input) {
	int cap = 8;
	int len = 0;
	struct name *list = malloc(sizeof(*list)*cap);

	const char *const saved_input = input;
	while (*input != '\0') {
		if (len >= cap) {
			cap *= 2;
			list = realloc(list, sizeof(*list)*cap);
		}
		list[len++] = parse_name(&input);
		assert_string(&input, ": ");

		for (;;) {
			if (len >= cap) {
				cap *= 2;
				list = realloc(list, sizeof(*list)*cap);
			}
			list[len++] = parse_name(&input);
			if (*input == ' ') {
				input++;
			} else if (*input == '\n') {
				input++;
				break;
			} else if (*input == '\0') {
				break;
			} else {
				FAIL("parse error");
			}
		}
	}
	input = saved_input;

	qsort(list, len, sizeof(*list), namecmp);
	remove_duplicates(list, &len);

	struct adjacency_list result;
	adjacency_list_init(&result, len);
	
	while (*input != '\0') {
		struct name name1 = parse_name(&input);
		int i = name_idx(list, len, name1);
		assert_string(&input, ": ");
		for (;;) {
			struct name name2 = parse_name(&input);
			int j = name_idx(list, len, name2);

			adjacency_list_add(&result, i, j);
			
			if (*input == ' ') {
				input++;
			} else if (*input == '\n') {
				input++;
				break;
			} else if (*input == '\0') {
				break;
			} else {
				FAIL("parse error");
			}
		}
	}

	free(list);
	return result;
}

struct queue_element {
	int node;

	int *path;
	int pathlen;
};

struct queue {
	int front;
	int tail;
	struct queue_element elements[QUEUE_MAX_SIZE];
};

static void queue_init(struct queue *q) {
	q->front = 0;
	q->tail = 0;
}

static void queue_push(struct queue *q, struct queue_element s) {
	q->elements[q->tail] = s;
	q->tail++;
	q->tail %= QUEUE_MAX_SIZE;
	ASSERT(q->tail != q->front, "queue full");
}

static struct queue_element queue_pop(struct queue *q) {
	ASSERT(q->tail != q->front, "queue empty");
	struct queue_element s = q->elements[q->front];
	q->front++;
	q->front %= QUEUE_MAX_SIZE;
	return s;
}

static bool queue_empty(const struct queue *q) {
	return q->front == q->tail;
}

static int *bfs(int from, int to, const struct adjacency_list *adjacency_list, int *pathlen) {
	bool *seen = malloc(sizeof(*seen)*adjacency_list->len);
	memset(seen, 0, sizeof(*seen)*adjacency_list->len);
	
	struct queue_element e;
	e.node = from;
	e.path = malloc(sizeof(*e.path));
	e.path[0] = from;
	e.pathlen = 1;

	struct queue q;
	queue_init(&q);
	queue_push(&q, e);

	bool found = false;
	while (!queue_empty(&q)) {
		e = queue_pop(&q);
		if (e.node == to) {
			found = true;
			break;
		}
		if (seen[e.node]) {
			free(e.path);
			continue;
		}
		seen[e.node] = true;

		
		for (int next_i=0; next_i<adjacency_list_count_neighbors(adjacency_list, e.node); next_i++) {
			int next = adjacency_list_ith_neighbor(adjacency_list, e.node, next_i);
			if (next != -1 && !seen[next]) {
				struct queue_element n;
				n.node = next;
				n.pathlen = e.pathlen+1;
				n.path = malloc(sizeof(*n.path)*n.pathlen);
				for (int i=0; i<e.pathlen; i++) {
					n.path[i] = e.path[i];
				}
				n.path[e.pathlen] = next;
				queue_push(&q, n);
			}
		}
		free(e.path);
	}
	free(seen);

	if (!found) {
		return NULL;
	}

	while (!queue_empty(&q)) {
		struct queue_element n = queue_pop(&q);
		free(n.path);
	}

	if (pathlen != NULL) {
		*pathlen = e.pathlen;
	}
	return e.path;
}

static void solution1(const char *const input, char *const output) {
	struct adjacency_list adjacency_list = parse_input(input);

	int *group = malloc(sizeof(*group)*adjacency_list.len);

	int source = 0;
	group[source] = 1;
	for (int i=1; i<adjacency_list.len; i++) {
		DBG("%d/%d", i, adjacency_list.len);
		int *paths[3];
		int pathlens[3];
		for (int j=0; j<3; j++) {
			int pathlen;
			int *path = bfs(source, i, &adjacency_list, &pathlen);
			ASSERT(path != NULL, "disconnected graph");

			int prev = path[0];
			for (int k=1; k<pathlen; k++) {
				int next = path[k];
				adjacency_list_invalidate_neighbors(&adjacency_list, prev, next);
				prev = next;
			}
			paths[j] = path;
			pathlens[j] = pathlen;
		}

		int *path = bfs(source, i, &adjacency_list, NULL);
		if (path != NULL) {
			group[i] = 1;
		} else {
			group[i] = 2;
		}
		free(path);

		for (int j=0; j<3; j++) {
			path = paths[j];
			int pathlen = pathlens[j];
			
			int prev = path[0];
			for (int k=1; k<pathlen; k++) {
				int next = path[k];
				adjacency_list_revalidate_neighbors(&adjacency_list, prev, next);
				prev = next;
			}
			
			free(path);
		}
	}


	int count = 0;
	for (int i=0; i<adjacency_list.len; i++) {
		if (group[i] == 1) {
			count++;
		}
	}
	ASSERT(count < adjacency_list.len, "all nodes in same group?");
	
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count * (adjacency_list.len - count));
	adjacency_list_free(&adjacency_list);
	free(group);
}

static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "SNOWY CHRISTMAS ACHIEVED");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
