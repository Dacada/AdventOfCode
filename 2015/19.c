#include <aoclib.h>
#include <stdio.h>

#define UPPERCASE(c) ((c) >= 'A' && (c) <= 'Z')
#define LOWERCASE(c) ((c) >= 'a' && (c) <= 'z')
#define LETTER(c) (UPPERCASE(c) || LOWERCASE(c))

typedef struct pair_t {
  int from;
  int to[15];
} pair_t;

typedef struct kvp_t {
  int key;
  int value;
} kvp_t;

static pair_t pairs_arena[50];
static pair_t *pairs[50];
static int conversions_count = 0;

static kvp_t pair_indices[50];
static size_t pair_indices_size;

static int line[500];

static int cmp_pair(const void *pair1, const void *pair2) {
  int from1 = (*((const pair_t *const *const)pair1))->from;
  int from2 = (*((const pair_t *const *const)pair2))->from;

  if (from1 > from2) {
    return 1;
  } else if (from1 < from2) {
    return -1;
  } else {
    return 0;
  }
}

static int encode_molecule(const char *const text) {
  ASSERT(text[0] != '\0', "Can't encode empty molecule");
  if (text[1] == '\0') {
    return (int)text[0];
  } else {
    ASSERT(text[2] == '\0', "Molecule can't have more than 2 characters");
    return (int)text[0] | ((int)text[1] << 8);
  }
}

static void decode_molecule(int encoded, char *const text) {
  text[0] = (char)(encoded & 0xFF);
  text[1] = (char)((encoded & 0xFF00) >> 8);
}

static size_t parse_one_molecule(const char *const input, size_t i, char *const name) {
  ASSERT(UPPERCASE(input[i]) || input[i] == 'e', "Can't parse molecule if it doesn't start with uppercase");
  name[0] = input[i++];
  if (LOWERCASE(input[i])) {
    name[1] = input[i++];
    name[2] = '\0';
    return i;
  } else {
    name[1] = '\0';
    return i;
  }
}

static bool parse_next_molecule(const char *const input, size_t *const i, int *const encoded_result) {
  if (UPPERCASE(input[*i])) {
    char m[3];
    *i = parse_one_molecule(input, *i, m);
    *encoded_result = encode_molecule(m);
    return true;
  } else {
    *encoded_result = -1;
    return false;
  }
}

static size_t parse_line(const char *const input, size_t i) {
  char from[3];
  i = parse_one_molecule(input, i, from);
  pairs[conversions_count]->from = encode_molecule(from);
  ASSERT(input[i] == ' ', "Invalid separator in conversions");
  ASSERT(input[++i] == '=', "Invalid separator in conversions");
  ASSERT(input[++i] == '>', "Invalid separator in conversions");
  ASSERT(input[++i] == ' ', "Invalid separator in conversions");
  i++;

  int j = 0;
  while (parse_next_molecule(input, &i, &pairs[conversions_count]->to[j++]));

  conversions_count++;
  return i;
}

static size_t parse_last_line(const char *const input, size_t i) {
  int j = 0;
  i += 2;
  
  while (parse_next_molecule(input, &i, &line[j++]));
  
  return i;
}

static void parse(const char *const input) {
  size_t i;
  
  for (i=0;; i++) {
    i = parse_line(input, i);
    if (input[i+1] == '\n') {
      break;
    }
  }

  i = parse_last_line(input, i);
  ASSERT(input[i+1] == '\0', "Did not parse full input");
}

static void init_pairs(void) {
  for (int i=0; i<50; i++) {
    pairs[i] = &pairs_arena[i];
  }
}

static void prepare_pairs_indexing(void) {
  qsort(pairs, conversions_count, sizeof(pair_t*), cmp_pair);
  
  int prev = -1;
  int j = 0;
  for (int i=0; i<conversions_count; i++) {
    int c = pairs[i]->from;
    if (c != prev) {
      prev = c;
      pair_indices[j].key = c;
      pair_indices[j].value = i;

      j++;
    }
  }
  pair_indices[j].key = -1;
  pair_indices[j].value = -1;
  pair_indices_size = j;
}

static int kvp_cmp(const void *key_ptr, const void *element_ptr) {
  const int *const key = key_ptr;
  const kvp_t *const element = element_ptr;

  if (*key < element->key) {
    return -1;
  } else if (*key > element->key) {
    return 1;
  } else {
    return 0;
  }
}

static int line_cmp(const void *arr1_ptr, const void *arr2_ptr) {
  const int *const arr1 = arr1_ptr;
  const int *const arr2 = arr2_ptr;

  if (arr1[1] == -1) {
    if (arr2[1] == -1) {
      return 0;
    } else {
      return 1;
    }
  }
  else if (arr2[1] == -1) {
    return -1;
  }
  
  for (int i=1; i<500; i++) {
    if (arr1[i] < arr2[i]) return -1;
    else if (arr1[i] > arr2[i]) return 1;
  }

  return 0;
}

static int solve(void) {
  static int lines_arena[500][500];
  static int *lines[500];
  for (int i=0; i<500; i++) {
    lines_arena[i][1] = -1;
    lines[i] = lines_arena[i];
  }

  // Iterate elements of input line
  for (size_t i=0; i<500; i++) {
    int element = line[i];
    if (element == -1) break;
    
    // Find kvp in pair_indices, if it exists
    kvp_t *kvp = bsearch(&element, pair_indices, pair_indices_size, sizeof(kvp_t), kvp_cmp);
    if (kvp != NULL) {
      // This kvp points us to the start of the block of pairs that go from that element to others
      // For example, if element was Ca and this was part of list of elements:
      // ...
      // B => ...
      // B => ...
      // * Ca => ...
      // Ca => ...
      // Ca => ...
      // Ca => ...
      // O => ...
      // ...
      // The * line is where the index that kvp.value is points to
      // So now we iterate the list of pairs, starting from kvp.value, until the "from" value is different from kvp.key
      for (int j=kvp->value; j<50; j++) {
	pair_t *pair = pairs[j];
	if (pair->from != kvp->key) break;

	// https://stackoverflow.com/questions/7666509/hash-function-for-string
	unsigned int hash = 5381;

	// Now we copy the line, from the start to i (not included) into lines_arena[i] but skipping the first int
	// (and we build the hash while doing so)
	for (size_t m=0; m<i; m++) {
	  hash = ((hash << 5) + hash) + line[m]; /* hash * 33 + c */
	  lines_arena[i][m+1] = line[m];
	}
	
	// Next we copy the "to" value until the first -1 (and build the hash)
	int k;
	for (k=0; k<15; k++) {
	  if (pair->to[k] == -1) break;
	  hash = ((hash << 5) + hash) + pair->to[k];
	  lines_arena[i][i+1+k] = pair->to[k];
	}
	
	// Finally, we copy the rest of the line where we left off excluding the i position (and build the hash)
	for (size_t m=0; m<i; m++) {
	  hash = ((hash << 5) + hash) + line[i+1+m];
	  lines_arena[i][i+1+k+m] = line[i+1+m];
	  if (line[i+1+m] == -1) break;
	}

	// Now we put into the first int the hash of the line
	lines_arena[i][0] = hash;
      }
    }
  }

  // Sort the lines
  qsort(lines, 500, sizeof(int*), line_cmp);
  // TODO: For some reason lines is not sorted properly

  // Count different lines, only need to compare with previous since they're sorted
  int res = 1;
  int *prev = lines[0];
  for (int i=1; i<500; i++) {
    if (lines[i][1] == -1) break;
    
    int *curr = lines[i];
    if (curr[0] != prev[0]) { // hash comparison, if it's different they're definitely different
      res++;
    } else { // else they might still be different
      if (line_cmp(curr, prev) != 0) // this still does one extra comparison (hash values again) but come on
	res ++;
    }
    prev = curr;
  }

  return res;
}

static void solution1(const char *const input, char *const output) {
  init_pairs();
  parse(input);
  prepare_pairs_indexing();

  int versions = solve();

  char m[3], m2[3];
  for (size_t i=0; i<pair_indices_size; i++) {
    int from = pair_indices[i].key;
    decode_molecule(from, m);
    fprintf(stderr, "%s:\n", m);
    for (int j=pair_indices[i].value; j<50; j++) {
      pair_t *pair = pairs[j];
      if (pair->from != from) break;
      fprintf(stderr, "  %s =>", m);
      for (int k=0; k<15; k++) {
	int to = pair->to[k];
	if (to == -1) break;
	decode_molecule(to, m2);
	fprintf(stderr, "%s", m2);
      }
      fprintf(stderr, "\n");
    }
  }
  
  fprintf(stderr, "\n");
  for (int i=0; i<500; i++) {
    if (line[i] == -1) break;
    decode_molecule(line[i], m);
    fprintf(stderr, "%s", m);
  }
  fprintf(stderr, "\n");
  
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", versions);
}

static void solution2(const char *const input, char *const output) {
  (void)input;
  snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
