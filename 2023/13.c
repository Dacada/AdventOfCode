#include <aoclib.h>
#include <stdio.h>
#include <string.h>

#define IDX(pattern, x, y) ((pattern)->str[(x) + (y) * (pattern)->width])

struct pattern {
  char *str;
  int width;
  int height;
};

static void parse_pattern(const char **input, struct pattern *pattern) {
  int cap = 16;
  int len = 0;
  char *list = malloc(sizeof(*list) * cap);

  int width = 0;
  int height = 0;
  for (;;) {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }

    char c = **input;
    if (c == '\0') {
      break;
    }
    if (c == '\n') {
      *input += 1;
      height++;
      if (**input == '\n' || **input == '\0') {
        break;
      }
      width = 0;
      continue;
    }
    list[len++] = c;
    width += 1;
    *input += 1;
  }

  if (**input == '\n') {
    *input += 1;
  }

  pattern->str = list;
  pattern->width = width;
  pattern->height = height;
}

static struct pattern *parse_input(const char *input, int *length) {
  int cap = 1;
  int len = 0;
  struct pattern *list = malloc(sizeof(*list) * cap);

  while (*input != '\0') {
    if (len >= cap) {
      cap *= 2;
      list = realloc(list, sizeof(*list) * cap);
    }
    parse_pattern(&input, list + len);
    len++;
  }

  *length = len;
  return list;
}

static bool verify_reflection_vertical(const struct pattern *pattern, int x) {
  char *buff1 = malloc(pattern->height);
  char *buff2 = malloc(pattern->height);

  int i1 = x - 1;
  int i2 = x;

  bool result = true;
  while (i1 >= 0 && i2 < pattern->width) {
    int buff1_i = 0;
    int buff2_i = 0;
    for (int j = 0; j < pattern->height; j++) {
      buff1[buff1_i++] = IDX(pattern, i1, j);
      buff2[buff2_i++] = IDX(pattern, i2, j);
    }
    DBG("'%.*s' '%.*s' %d", pattern->height, buff1, pattern->height, buff2, strncmp(buff1, buff2, pattern->height));
    if (strncmp(buff1, buff2, pattern->height) != 0) {
      result = false;
      goto end;
    }
    i1--;
    i2++;
  }

end:
  free(buff1);
  free(buff2);
  DBG("res: %d", result);
  return result;
}

static bool verify_reflection_horizontal(const struct pattern *pattern, int y) {
  char *buff1 = malloc(pattern->width);
  char *buff2 = malloc(pattern->width);

  int j1 = y - 1;
  int j2 = y;

  bool result = true;
  while (j1 >= 0 && j2 < pattern->height) {
    int buff1_i = 0;
    int buff2_i = 0;
    for (int i = 0; i < pattern->width; i++) {
      buff1[buff1_i++] = IDX(pattern, i, j1);
      buff2[buff2_i++] = IDX(pattern, i, j2);
    }
    DBG("'%.*s' '%.*s' %d", pattern->width, buff1, pattern->width, buff2, strncmp(buff1, buff2, pattern->width));
    if (strncmp(buff1, buff2, pattern->width) != 0) {
      result = false;
      goto end;
    }
    j1--;
    j2++;
  }

end:
  free(buff1);
  free(buff2);
  DBG("res: %d", result);
  return result;
}

static int get_reflection_vertical(const struct pattern *pattern, int forbidden_value) {
  char *curr = malloc(pattern->height);
  char *prev = malloc(pattern->height);

  prev[0] = '\0';

  int result = 0;
  for (int i = 0; i < pattern->width; i++) {
    int curr_i = 0;
    for (int j = 0; j < pattern->height; j++) {
      curr[curr_i++] = IDX(pattern, i, j);
    }
    DBG("'%.*s' '%.*s' %d", pattern->height, curr, pattern->height, prev, strncmp(curr, prev, pattern->height));
    if (i != forbidden_value && strncmp(curr, prev, pattern->height) == 0 && verify_reflection_vertical(pattern, i)) {
      result = i;
      goto end;
    }
    char *tmp = prev;
    prev = curr;
    curr = tmp;
  }

end:
  free(prev);
  free(curr);
  DBG("res: %d", result);
  return result;
}

static int get_reflection_horizontal(const struct pattern *pattern, int forbidden_value) {
  char *curr = malloc(pattern->width);
  char *prev = malloc(pattern->width);

  prev[0] = '\0';

  int result = 0;
  for (int j = 0; j < pattern->height; j++) {
    int curr_i = 0;
    for (int i = 0; i < pattern->width; i++) {
      curr[curr_i++] = IDX(pattern, i, j);
    }
    DBG("'%.*s' '%.*s' %d", pattern->width, curr, pattern->width, prev, strncmp(curr, prev, pattern->width));
    if (j != forbidden_value && strncmp(curr, prev, pattern->width) == 0 && verify_reflection_horizontal(pattern, j)) {
      result = j;
      goto end;
    }
    char *tmp = prev;
    prev = curr;
    curr = tmp;
  }

end:
  free(prev);
  free(curr);
  DBG("res: %d", result);
  return result;
}

static int get_reflection(const struct pattern *pattern, int forbidden_value) {
  int forbidden = -1;
  if (forbidden_value > 0 && forbidden_value < 100) {
    forbidden = forbidden_value;
  }
  int n = get_reflection_vertical(pattern, forbidden);
  if (n > 0) {
    return n;
  }

  forbidden = -1;
  if (forbidden_value > 0 && forbidden_value >= 100) {
    forbidden = forbidden_value / 100;
  }
  n = get_reflection_horizontal(pattern, forbidden);
  if (n > 0) {
    return n * 100;
  }

  return -1;
}

static void get_reflections(const struct pattern *patterns, int npatterns, int *reflections) {
  for (int i = 0; i < npatterns; i++) {
    const struct pattern *pattern = patterns + i;
    reflections[i] = get_reflection(pattern, -1);
  }
}

static void solution1(const char *const input, char *const output) {
  int npatterns;
  struct pattern *patterns = parse_input(input, &npatterns);

#ifdef DEBUG
  for (int x = 0; x < npatterns; x++) {
    struct pattern *pattern = patterns + x;
    for (int j = 0; j < pattern->height; j++) {
      for (int i = 0; i < pattern->width; i++) {
        fputc(IDX(pattern, i, j), stderr);
      }
      fputc('\n', stderr);
    }
    fputc('\n', stderr);
  }
#endif

  int *reflections = malloc(sizeof(*reflections) * npatterns);
  get_reflections(patterns, npatterns, reflections);

  int res = 0;
  for (int i = 0; i < npatterns; i++) {
    res += reflections[i];
    free(patterns[i].str);
  }

  free(patterns);
  free(reflections);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

struct alter_iterator {
  int j;
  int i;
  char c;
  struct pattern *pattern;
};

static char alter_pattern(struct pattern *pattern, int x, int y) {
  char c = IDX(pattern, x, y);
  if (c == '.') {
    IDX(pattern, x, y) = '#';
  } else {
    IDX(pattern, x, y) = '.';
  }
  return c;
}

static struct alter_iterator make_alter_iterator(struct pattern *pattern) {
  struct alter_iterator iter;
  iter.j = 0;
  iter.i = 0;
  iter.c = alter_pattern(pattern, 0, 0);
  iter.pattern = pattern;
  return iter;
}

static struct pattern *get_next_altered_pattern(struct alter_iterator *iter) {
  IDX(iter->pattern, iter->i, iter->j) = iter->c;
  iter->i++;
  if (iter->i >= iter->pattern->width) {
    iter->i = 0;
    iter->j++;
    if (iter->j >= iter->pattern->height) {
      return NULL;
    }
  }
  iter->c = alter_pattern(iter->pattern, iter->i, iter->j);
  return iter->pattern;
}

static void solution2(const char *const input, char *const output) {
  int npatterns;
  struct pattern *patterns = parse_input(input, &npatterns);

  int *reflections = malloc(sizeof(*reflections) * npatterns);
  get_reflections(patterns, npatterns, reflections);

  int res = 0;
  for (int i = 0; i < npatterns; i++) {
    DBG("i=%d", i);
    int previous_reflection = reflections[i];
    struct alter_iterator iter = make_alter_iterator(&patterns[i]);
    struct pattern *altered_pattern;
    while ((altered_pattern = get_next_altered_pattern(&iter)) != NULL) {
      int n = get_reflection(altered_pattern, previous_reflection);
      if (n > 0) {
        res += n;
        break;
      }
    }
    free(patterns[i].str);
  }

  free(patterns);
  free(reflections);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", res);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
