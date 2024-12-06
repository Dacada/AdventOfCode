#include <aoclib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static bool val_year(const char *const content, const size_t len, int min, int max) {
  if (len != 4) {
    return false;
  }

  int n = 0;
  for (size_t i = 0; i < len; i++) {
    char c = content[i];
    if (c < '0' || c > '9') {
      return false;
    }
    n = n * 10 + c - '0';
  }

  return n >= min && n <= max;
}

enum fields { byr, iyr, eyr, hgt, hcl, ecl, pid, cid, last };
char *fieldsstr[last] = {"byr", "iyr", "eyr", "hgt", "hcl", "ecl", "pid", "cid"};
bool (*val_funs[last])(const char *, size_t);

static bool val_noop(const char *const content, const size_t len) {
  (void)content;
  (void)len;
  return true;
}
static bool val_byr(const char *const content, const size_t len) { return val_year(content, len, 1920, 2002); }
static bool val_iyr(const char *const content, const size_t len) { return val_year(content, len, 2010, 2020); }
static bool val_eyr(const char *const content, const size_t len) { return val_year(content, len, 2020, 2030); }
static bool val_hgt(const char *const content, const size_t len) {
  int n = 0;
  for (size_t i = 0; i < len - 2; i++) {
    char c = content[i];
    if (c < '0' || c > '9') {
      return false;
    }
    n = n * 10 + c - '0';
  }

  char unit[3];
  strncpy(unit, content + len - 2, 2);
  if (strncmp(unit, "cm", 2) == 0) {
    return n >= 150 && n <= 193;
  } else if (strncmp(unit, "in", 2) == 0) {
    return n >= 59 && n <= 76;
  } else {
    return false;
  }
}
static bool val_hcl(const char *const content, const size_t len) {
  if (len != 7) {
    return false;
  }
  if (content[0] != '#') {
    return false;
  }
  for (size_t i = 1; i < len; i++) {
    char c = content[i];
    if ((c < '0' || c > '9') && (c < 'a' || c > 'f')) {
      return false;
    }
  }
  return true;
}
static bool val_ecl(const char *const content, const size_t len) {
  if (len != 3) {
    return false;
  }
  return strncmp(content, "amb", len) == 0 || strncmp(content, "blu", len) == 0 || strncmp(content, "brn", len) == 0 ||
         strncmp(content, "gry", len) == 0 || strncmp(content, "grn", len) == 0 || strncmp(content, "hzl", len) == 0 ||
         strncmp(content, "oth", len) == 0;
}
static bool val_pid(const char *const content, const size_t len) {
  if (len != 9) {
    return false;
  }
  for (size_t i = 0; i < len; i++) {
    char c = content[i];
    if (c < '0' || c > '9') {
      return false;
    }
  }
  return true;
}

static int count_valid(const char *const input) {
  int result = 0;
  for (size_t i = 0;; i++) {
    bool found[last];
    memset(found, 0, sizeof(found));

    while (input[i] != '\n' && input[i] != '\0') {
      char field[4];
      strncpy(field, input + i, 3);
      i += 3;
      ASSERT(input[i++] == ':', "invalid input");

      char content[256];
      int j = 0;
      while (!isspace(input[i + j])) {
        content[j] = input[i + j];
        j++;
        ASSERT(j < 256, "content too large");
      }
      i += j + 1;

      for (enum fields f = byr; f < last; f++) {
        if (strncmp(field, fieldsstr[f], 3) == 0) {
          found[f] = val_funs[f](content, j);
          break;
        }
      }
    }

    bool allfound = true;
    for (enum fields f = byr; f < cid; f++) {
      if (!found[f]) {
        allfound = false;
        break;
      }
    }

    if (allfound) {
      result++;
    }

    if (input[i] == '\0') {
      break;
    }
  }
  return result;
}

static void solution1(const char *const input, char *const output) {
  for (enum fields f = byr; f < last; f++) {
    val_funs[f] = val_noop;
  }

  int result = count_valid(input);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

static void solution2(const char *const input, char *const output) {
  val_funs[byr] = val_byr;
  val_funs[iyr] = val_iyr;
  val_funs[eyr] = val_eyr;
  val_funs[hgt] = val_hgt;
  val_funs[hcl] = val_hcl;
  val_funs[ecl] = val_ecl;
  val_funs[pid] = val_pid;
  val_funs[cid] = val_noop;

  int result = count_valid(input);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%d", result);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
