#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_MOLECULES 20

struct compound {
        int *molecules;
        size_t size;
};

struct compoundlist {
        struct compound *compounds;
        size_t size, current;
};

struct moleculetree {
        int value;
        size_t nextcount, nextcap;
        struct moleculetree *next;
};

static void tree_init(struct moleculetree *tree) {
        tree->value = -2;
        tree->nextcount = 0;
        tree->nextcap = 0;
        tree->next = NULL;
}
static struct moleculetree *add_to_tree(struct moleculetree *head, int molecule) {
        if (head->nextcount > 0) {
                for (size_t i=0; i<head->nextcount; i++) {
                        if (head->next[i].value == molecule) {
                                return &head->next[i];
                        }
                }
        }
        
        if (head->nextcap == 0) {
                head->nextcap = 4;
                head->next = malloc(head->nextcap * sizeof *head->next);
        }

        while (head->nextcount >= head->nextcap) {
                head->nextcap *= 2;
                void *tmp = realloc(head->next, head->nextcap * sizeof *head->next);
                if (tmp == NULL) {
                        free(head->next);
                        FAIL("memory error");
                }
                head->next = tmp;
        }

        struct moleculetree *new = &head->next[head->nextcount++];
        new->value = molecule;
        new->nextcount = new->nextcap = 0;
        new->next = NULL;
        return new;
}
static bool finish_tree_branch(struct moleculetree *head) {
        size_t oldcount = head->nextcount;
        add_to_tree(head, -1);
        size_t newcount = head->nextcount;

        if (oldcount != newcount) {
                DBG("Finished: Tree count ++");
                return true;
        } else {
                DBG("Finished: Repeat ending token");
                return false;
        }
}
static void tree_free(struct moleculetree *head) {
        for (size_t i=0; i<head->nextcount; i++) {
                tree_free(&head->next[i]);
        }
        free(head->next);
}

static void add_association(struct compoundlist *associations, int molecule, struct compound compound) {
        struct compoundlist *list = &associations[molecule];
        while (list->size <= list->current) {
                if (list->size == 0)
                        list->size = 1;
                else
                        list->size *= 2;
                void *tmp = realloc(list->compounds, list->size * sizeof *list->compounds);
                if (tmp == NULL) {
                        free(list->compounds);
                        FAIL("memory error");
                }
                list->compounds = tmp;
        }
        list->compounds[list->current++] = compound;
}

static int molecules_len = 0;
static char molecules[MAX_MOLECULES*2];
static int mol2num(const char *const molecule) {
        for (int i=0; i<molecules_len; i+=2) {
                if (molecule[0] == molecules[i] &&
                    molecule[1] == molecules[i+1]) {
                        return i/2;
                }
        }

        int j = molecules_len;
        molecules[j] = molecule[0];
        molecules[j+1] = molecule[1];
        molecules_len += 2;
        return j/2;
}
#ifdef DEBUG
static void num2mol(int n, char m[2]) {
        m[0] = molecules[n*2];
        m[1] = molecules[n*2+1];
}
#endif

static int parse_molecule(const char *const input, size_t *const i) {
        char molecule[2];
        molecule[1] = 0;

        ASSERT(isalpha(input[*i]), "Can't parse empty molecule");
        if (!isalpha(input[*i+1])) {
                molecule[0] = input[*i];
                ++*i;
        } else if (isupper(input[*i]) && isupper(input[*i+1])) {
                molecule[0] = input[*i];
                ++*i;
        } else if (isupper(input[*i])) {
                molecule[0] = input[*i];
                molecule[1] = input[*i+1];
                *i += 2;
        } else {
                ASSERT(isupper(input[*i+1]), "Unexpected input");
                molecule[0] = input[*i];
                ++*i;
        }

        return mol2num(molecule);
}

static struct compound parse_compound(const char *const input, size_t *const i) {
        size_t j = 0;
        size_t size = 4;
        int *mols = malloc(size * sizeof *mols);
        ASSERT(mols != NULL, "Memory error");

        while (isalpha(input[*i])) {
                while (j >= size) {
                        size *= 2;
                        void *tmp = realloc(mols, size * sizeof *mols);
                        if (tmp == NULL) {
                                free(mols);
                                FAIL("Memory error");
                        }
                        mols = tmp;
                }
                mols[j++] = parse_molecule(input, i);
        }

        struct compound c;
        c.molecules = realloc(mols, j * sizeof *mols);
        ASSERT(c.molecules != NULL, "Memory error");
        c.size = j;
        return c;
}

static void parse_line(struct compoundlist *associations, const char *const input, size_t *i) {
        int molecule = parse_molecule(input, i);
        ASSERT(input[(*i)++] == ' ', "Expected separator");
        ASSERT(input[(*i)++] == '=', "Expected separator");
        ASSERT(input[(*i)++] == '>', "Expected separator");
        ASSERT(input[(*i)++] == ' ', "Expected separator");
        struct compound result = parse_compound(input, i);
        add_association(associations, molecule, result);
}

static struct compound parse(struct compoundlist *associations, const char *const input) {
        size_t i;
        for (i=0;; i++) {
                if (input[i] == '\n') {
                        break;
                }
                parse_line(associations, input, &i);
        }

        i++;
        return parse_compound(input, &i);
}

static void solution1(const char *const input, char *const output) {
        static struct compoundlist associations[MAX_MOLECULES];
        struct compound medicine = parse(associations, input);

#ifdef DEBUG
        fprintf(stderr, "Medicine:\n");
        for (size_t i=0; i<medicine.size; i++) {
                int mol = medicine.molecules[i];
                char molstr[3];
                molstr[2] = 0;
                num2mol(mol, molstr);
                fprintf(stderr, "%s ", molstr);
        }
        fprintf(stderr, "\n\n");

        fprintf(stderr, "Compounds:\n");
        for (size_t i=0; i<MAX_MOLECULES; i++) {
                struct compoundlist *list = &associations[i];
                if (list->current > 0) {
                        char mstr[3];
                        mstr[2] = 0;
                        num2mol(i, mstr);
                        fprintf(stderr, "%s => ", mstr);
                        for (size_t j=0; j<list->current; j++) {
                                struct compound *comp = &list->compounds[j];
                                for (size_t k=0; k<comp->size; k++) {
                                        int mol = comp->molecules[k];
                                        char molstr[3];
                                        molstr[2] = 0;
                                        num2mol(mol, molstr);
                                        fprintf(stderr, "%s ", molstr);
                                }
                                fprintf(stderr, "\n     ");
                                if (mstr[1] != 0) {
                                        fprintf(stderr, " ");
                                }
                        }
                        fprintf(stderr, "\n");
                }
        }
#endif
        
        static struct moleculetree tree;
        int tree_count = 0;
        
        tree_init(&tree);
        // for every molecule in the medicine (i)
        for (size_t i=0; i<medicine.size; i++) {
                int molecule = medicine.molecules[i];
                
#ifdef DEBUG
                char molecule_str[3]; molecule_str[2] = 0;
                num2mol(molecule, molecule_str);
                DBG("Checking %s...", molecule_str);
#endif
                
                struct compoundlist *replacements = &associations[molecule];
                // if it has replacements
                if (replacements->current > 0) {
                        DBG("Has replacements");
                        
                        // for every replacement for that molecule
                        for (size_t j=0; j<replacements->current; j++) {

#ifdef DEBUG
                                fprintf(stderr, "Checking replacement ");
                                struct compound *cmp = &replacements->compounds[j];
                                for (size_t c=0; c<cmp->size; c++) {
                                        char s[3]; s[2] = 0;
                                        num2mol(cmp->molecules[c], s);
                                        fprintf(stderr, "%s", s);
                                }
                                fprintf(stderr, "...\n");
#endif
                                
                                // take every molecule in the medicine again (k)
                                struct moleculetree *head = &tree;
                                for (size_t k=0; k<medicine.size; k++) {
                                        // if it's the one we're at with i
                                        if (i == k) { 
                                                DBG("Adding to tree %s's replacement:", molecule_str);
                                                struct compound *compound = &replacements->compounds[j];
                                                // for every molecule in compound
                                                for (size_t l=0; l<compound->size; l++) {
#ifdef DEBUG
                                                        char c[3]; c[2] = 0;
                                                        num2mol(compound->molecules[l], c);
                                                        DBG("%s", c);
#endif
                                                        head = add_to_tree(head, compound->molecules[l]);
                                                }
                                        // otherwise
                                        } else {
#ifdef DEBUG
                                                char mstr[3]; mstr[2] = 0;
                                                num2mol(medicine.molecules[k], mstr);
                                                //DBG("Adding to tree %s", mstr);
#endif
                                                head = add_to_tree(head, medicine.molecules[k]);
                                        }
                                }
                                if (finish_tree_branch(head)) {
                                        tree_count++;
                                }
                        }
                } else {
                        DBG("Has no replacements, skipping.");
                }
        }
        
        free(medicine.molecules);
        for (size_t i=0; i<MAX_MOLECULES; i++) {
                struct compoundlist *list = &associations[i];
                if (list->current > 0) {
                        for (size_t j=0; j<list->current; j++) {
                                free(list->compounds[j].molecules);
                        }
                }
                free(list->compounds);
        }
        tree_free(&tree);

        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", tree_count);
}

static void solution2(const char *const input, char *const output) {
        (void)input;
        snprintf(output, OUTPUT_BUFFER_SIZE, "NOT SOLVED");
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
