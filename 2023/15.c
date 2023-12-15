#include <aoclib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* Today I'm doing object oriented programming in C */
/* This problem is just asking for it and I felt like it */
/* WARNING: There are iterators here */

struct hash {
	int value;
};

static void hash_init(struct hash *this) {
	this->value = 0;
}
static void hash_add(struct hash *this, char c) {
	this->value += c;
	this->value *= 17;
	this->value %= 256;
}

static int hash_result(const struct hash *this) {
	return this->value;
}

static void solution1(const char *const input, char *const output) {
	int res = 0;
	
	struct hash hash;
	hash_init(&hash);
	for (int i=0;; i++) {
		char c = input[i];
		if (c == '\0') {
			break;
		} else if (c == '\n') {
		} else if (c == ',') {
			res += hash_result(&hash);
			hash_init(&hash);
		} else {
			hash_add(&hash, c);
		}
	}
	res += hash_result(&hash);
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

struct charstring {
	int cap;
	int len;
	char *str;
};

struct charstringiterator {
	int i;
	const struct charstring *str;
};

static void charstring_init(struct charstring *this) {
	this->cap = 2;
	this->len = 0;
	this->str = malloc(sizeof(*this->str)*this->cap);
}

static void charstring_init_from(struct charstring *this, const struct charstring *other) {
	this->cap = other->cap;
	this->len = other->len;
	this->str = malloc(sizeof(*this->str)*this->cap);
	memcpy(this->str, other->str, other->len);
}

static void charstring_free(struct charstring *this) {
	free(this->str);
}

#ifdef DEBUG
static const char *charstring_tmpstr(const struct charstring *this) {
	static char tmpstr[256];
	for (int i=0; i<this->len; i++) {
		tmpstr[i] = this->str[i];
	}
	tmpstr[this->len] = '\0';
	return tmpstr;
}
#endif

static void charstring_add(struct charstring *this, char c) {
	if (this->len >= this->cap) {
		this->cap *= 2;
		this->str = realloc(this->str, sizeof(*this->str)*this->cap);
	}
	this->str[this->len] = c;
	this->len += 1;
}

static void charstring_clear(struct charstring *this) {
	this->len = 0;
}

static struct charstringiterator charstring_get_iterator(const struct charstring *this) {
	struct charstringiterator iter;
	iter.i = 0;
	iter.str = this;
	return iter;
}

static char *charstringiterator_next(struct charstringiterator *this, char *next) {
	if (this->i >= this->str->len) {
		return NULL;
	}
	*next = this->str->str[this->i];
	this->i += 1;
	return next;
}

static bool charstring_equal(const struct charstring *this, const struct charstring *other) {
	if (this->len != other->len) {
		return false;
	}
	return strncmp(this->str, other->str, this->len) == 0;
}

struct lens {
	struct charstring label;
	int focal_length;
	struct lens *next;
	struct lens *prev;
};

static void lens_init(struct lens *this, const struct charstring *label, int focal_length) {
	charstring_init_from(&this->label, label);
	this->focal_length = focal_length;
	this->next = NULL;
	this->prev = NULL;
}

static void lens_free(struct lens *this) {
	charstring_free(&this->label);
}

struct box {
	struct lens *first;
	struct lens *last;
};

struct boxiterator {
	struct lens *current;
};

struct boxconstiterator {
	const struct lens *current;
};

static void box_init(struct box *this) {
	this->first = NULL;
	this->last = NULL;
}

static void box_free(struct box *this) {
	struct lens *lens = this->first;

	while (lens != NULL) {
		struct lens *tmp = lens->next;
		lens_free(lens);
		free(lens);
		lens = tmp;
	}
}

static struct boxiterator box_get_iterator(struct box *this) {
	struct boxiterator iter;
	iter.current = this->first;
	return iter;
}

static struct lens *boxiterator_next(struct boxiterator *this) {
	if (this->current == NULL) {
		return NULL;
	}
	struct lens *lens = this->current;
	this->current = this->current->next;
	return lens;
}

static struct boxconstiterator box_get_const_iterator(const struct box *this) {
	struct boxconstiterator iter;
	iter.current = this->first;
	return iter;
}

static const struct lens *boxconstiterator_next(struct boxconstiterator *this) {
	if (this->current == NULL) {
		return NULL;
	}
	const struct lens *lens = this->current;
	this->current = this->current->next;
	return lens;
}


__attribute__((pure))
static struct lens *box_get_lens(struct box *this, const struct charstring *label) {
	struct boxiterator iter = box_get_iterator(this);

	struct lens *lens;
	while ((lens = boxiterator_next(&iter)) != NULL) {
		if (charstring_equal(&lens->label, label)) {
			return lens;
		}
	}

	return lens;
}

static void box_remove(struct box *this, const struct charstring *label) {
	struct lens *lens = box_get_lens(this, label);
	if (lens != NULL) {
		if (lens->prev != NULL) {
			lens->prev->next = lens->next;
		}
		if (lens->next != NULL) {
			lens->next->prev = lens->prev;
		}
		if (this->first == lens) {
			this->first = lens->next;
		}
		if (this->last == lens) {
			this->last = lens->prev;
		}
		lens_free(lens);
		free(lens);
	}
}

static void box_add_new(struct box *this, const struct charstring *label, int focal_length) {
	struct lens *new = malloc(sizeof(*new));
	lens_init(new, label, focal_length);
	if (this->first == NULL) {
		this->first = new;
		this->last = new;
	} else {
		this->last->next = new;
		new->prev = this->last;
		this->last = new;
	}
}

static void box_add(struct box *this, const struct charstring *label, int focal_length) {
	struct lens *lens = box_get_lens(this, label);
	if (lens == NULL) {
		box_add_new(this, label, focal_length);
	} else {
		lens->focal_length = focal_length;
	}

}

struct hashmap {
	struct box boxes[256];
};

struct hashmapiterator {
	int i;
	const struct hashmap *hashmap;
};

static void hashmap_init(struct hashmap *this) {
	for (int i=0; i<256; i++) {
		box_init(this->boxes + i);
	}
}

static void hashmap_free(struct hashmap *this) {
	for (int i=0; i<256; i++) {
		box_free(this->boxes + i);
	}
}

static struct hashmapiterator hashmap_get_iterator(const struct hashmap *this) {
	struct hashmapiterator iter;
	iter.i = 0;
	iter.hashmap = this;
	return iter;
}

static const struct box *hashmapiterator_next(struct hashmapiterator *this) {
	if (this->i >= 256) {
		return NULL;
	}
	const struct box *box = this->hashmap->boxes + this->i;
	this->i += 1;
	return box;
}

static struct box *hashmap_get_box(struct hashmap *this, const struct charstring *label) {
	struct hash hash;
	hash_init(&hash);
	struct charstringiterator iter = charstring_get_iterator(label);

	char c;
	while (charstringiterator_next(&iter, &c) != NULL) {
		hash_add(&hash, c);
	}
	int box = hash_result(&hash);

	return this->boxes + box;
}

static void hashmap_remove(struct hashmap *this, const struct charstring *label) {
	struct box *box = hashmap_get_box(this, label);
	box_remove(box, label);
}

static void hashmap_add(struct hashmap *this, const struct charstring *label, int focal_length) {
	struct box *box = hashmap_get_box(this, label);
	box_add(box, label, focal_length);
}

static void hashmap_operation(struct hashmap *hashmap, const struct charstring *label, char operation, int focal_length) {
	DBG("Operation %c for label %s with focal length %d", operation, charstring_tmpstr(label), focal_length);
	if (operation == '-') {
		hashmap_remove(hashmap, label);
	} else if (operation == '=') {
		hashmap_add(hashmap, label, focal_length);
	} else {
		FAIL("invalid operation %c", operation);
	}
}

static int hashmap_result(const struct hashmap *hashmap) {
	int res = 0;
	
	int i=0;
	const struct box *box;
	struct hashmapiterator hashmap_iter = hashmap_get_iterator(hashmap);
	while ((box = hashmapiterator_next(&hashmap_iter)) != NULL) {
		int j = 0;
		const struct lens *lens;
		struct boxconstiterator box_iter = box_get_const_iterator(box);
		while ((lens = boxconstiterator_next(&box_iter)) != NULL) {
			int r = (i+1) * (j+1) * lens->focal_length;
			DBG("'%s': '%d' (box %d) * '%d' (%d slot) * '%d' (focal length) = '%d'",
			    charstring_tmpstr(&lens->label), i+1, i, j+1, j+1, lens->focal_length, r);
			res += r;
			j++;
		}
		i++;
	}

	return res;
}

static void solution2(const char *const input, char *const output) {
	struct hashmap *hashmap = malloc(sizeof(*hashmap));
	hashmap_init(hashmap);

	char operation = '\0';
	int focal_length = 0;
	struct charstring label;
	charstring_init(&label);
	
	for (int i=0;; i++) {
		char c = input[i];
		if (c == '\0') {
			break;
		} else if (c == ',' || c == '\n') {
			hashmap_operation(hashmap, &label, operation, focal_length);
			focal_length = 0;
			charstring_clear(&label);
		} else if (c == '-' || c == '=') {
			operation = c;
		} else if (isdigit(c)) {
			focal_length *= 10;
			focal_length += c - '0';
		} else {
			charstring_add(&label, c);
		}
	}

        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", hashmap_result(hashmap));
	charstring_free(&label);
	hashmap_free(hashmap);
	free(hashmap);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
