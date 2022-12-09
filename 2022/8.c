#include <aoclib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define IDX(x,y,n) ((x)*(n)+(y))

struct tree_neighbors {
        int count[10];
};

struct tree_info {
        struct tree_neighbors up;
        struct tree_neighbors left;
        struct tree_neighbors down;
        struct tree_neighbors right;
};

static int get_tree_score(int idx, int* trees, struct tree_info *info) {
        int tree = trees[idx];
        return info[idx].up.count[tree] * info[idx].down.count[tree] *
                info[idx].left.count[tree] * info[idx].right.count[tree];
}

static void send_line(int point, bool start, bool vertical,
                      int *trees, bool *visible, struct tree_info *info, int len) {
        int init;
        int end;
        int inc;
        if (start) {
                init = 0;
                end = len - 1;
                inc = 1;
        } else {
                init = len - 1;
                end = 0;
                inc = -1;
        }

        int max = -1;
        for (int x=init; x!=end; x+=inc) {
                int i, j;
                int pi, pj;
                if (vertical) {
                        j = x;
                        pj = x - inc;
                        pi = i = point;
                } else {
                        i = x;
                        pi = i - inc;
                        pj = j = point;
                }
                int idx = IDX(i,j,len);
                int tree = trees[idx];

                // for part 1
                if (tree > max) {
                        visible[idx] = true;
                        max = tree;
                }

                // for part 2
                int *line;
                if (start) {
                        if (vertical) {
                                line = info[idx].up.count;
                        } else {
                                line = info[idx].left.count;
                        }
                } else {
                        if (vertical) {
                                line = info[idx].down.count;
                        } else {
                                line = info[idx].right.count;
                        }
                }
                if (x == init) {  // outer edge always 0
                        memset(line, 0, 10*sizeof(*line));
                        continue;
                }
                if (x == init + inc) {  // next layer always 1
                        for (int k=0; k<10; k++) {
                                line[k] = 1;
                        }
                        continue;
                }
                
                // same as previous layer but adding 1 to any numbers strictly greater than previous tree
                // and resetting to 1 numbers smaller than or equal to previous tree
                int pidx = IDX(pi,pj,len);
                int *prevline;
                if (start) {
                        if (vertical) {
                                prevline = info[pidx].up.count;
                        } else {
                                prevline = info[pidx].left.count;
                        }
                } else {
                        if (vertical) {
                                prevline = info[pidx].down.count;
                        } else {
                                prevline = info[pidx].right.count;
                        }
                }
                memcpy(line, prevline, 10*sizeof(*line));
                int prevtree = trees[pidx];
                for (int k=prevtree+1; k<10; k++) {
                        line[k]++; 
                }
                for (int k=0; k<=prevtree; k++) {
                        line[k] = 1;
                }
        }
}

static int *parse_input(const char *const input, int *const len) {
        for (*len=0; input[*len] != '\n'; *len+=1);
        int *res = malloc(*len**len*sizeof(*res));
        for (int i=0; i<*len; i++) {
                for (int j=0; j<*len; j++) {
                        char c = input[IDX(i,j,*len+1)];
                        ASSERT(isdigit(c), "parse error");
                        res[IDX(i,j,*len)] = c - '0';
                }
        }
        return res;
}

static int send_lines(const char *const input, int **trees, bool **visible, struct tree_info **info) {
        int len;
        *trees = parse_input(input, &len);
        *visible = calloc(len*len, sizeof(**visible));
        *info = calloc(len*len, sizeof(**info));

        for (int i=0; i<len; i++) {
                send_line(i, true, false, *trees, *visible, *info, len);
                send_line(i, true, true, *trees, *visible, *info, len);
                send_line(i, false, false, *trees, *visible, *info, len);
                send_line(i, false, true, *trees, *visible, *info, len);
        }

        return len;
}

static void solution1(const char *const input, char *const output) {
        int *trees;
        bool *visible;
        struct tree_info *info;
        int len = send_lines(input, &trees, &visible, &info);
        
        int count = 0;
        for (int i=0; i<len; i++) {
                for (int j=0; j<len; j++) {
#ifdef DEBUG
                        const char *fmt;
                        if (visible[IDX(i,j,len)]) {
                                fmt = "\x1b[1m%d\x1b[0m";
                        } else {
                                fmt = "%d";
                        }
                        fprintf(stderr, fmt, trees[IDX(i,j,len)]);
#endif
                        count += visible[IDX(i,j,len)];
                }
#ifdef DEBUG
                fprintf(stderr, "\n");
#endif
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", count);
        free(trees);
        free(visible);
        free(info);
}

static void solution2(const char *const input, char *const output) {
        int *trees;
        bool *visible;
        struct tree_info *info;
        int len = send_lines(input, &trees, &visible, &info);
        
        int score = 0;
        for (int i=0; i<len; i++) {
                for (int j=0; j<len; j++) {
                        int idx = IDX(i,j,len);
                        int s = get_tree_score(idx, trees, info);
                        if (s > score) {
                                score = s;
                        }
                }
        }
        
        snprintf(output, OUTPUT_BUFFER_SIZE, "%d", score);
        free(trees);
        free(visible);
        free(info);
}

int main(int argc, char *argv[]) {
        return aoc_run(argc, argv, solution1, solution2);
}
