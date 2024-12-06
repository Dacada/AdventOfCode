#include <aoclib.h>
#include <bsd/md5.h>
#include <stdio.h>
#include <string.h>

#define HASHES 1000

#define hexidx(c) (((c) <= '9') ? ((c) - '0') : ((c) - 'a' + 10))

struct idxinfo {
  char first_triple;
  bool has_quintuple[16];
};

struct idxinfoqueue {
  struct idxinfo elements[HASHES];
  size_t first;
};

static size_t num2str(size_t num, uint8_t *const str) {
  size_t len = 0;
  while (num != 0) {
    str[len++] = '0' + num % 10;
    num /= 10;
  }
  for (size_t i = 0; i < len / 2; i++) {
    ASSERT(i < 16, "logic error");
    size_t j = len - i - 1;
    ASSERT(j < 16, "logic error");
    char tmp = str[i];
    str[i] = str[j];
    str[j] = tmp;
  }
  return len;
}

static void hash_first(size_t idx, const unsigned char *const salt,
                       size_t salt_len, char *digest) {
  MD5_CTX ctx;
  MD5Init(&ctx);
  MD5Update(&ctx, salt, salt_len);

  unsigned char buff[16];
  size_t len = num2str(idx, buff);
  MD5Update(&ctx, buff, len);

  MD5End(&ctx, digest);
}

static void hash_second(size_t idx, const unsigned char *const salt,
                        size_t salt_len, char *digest) {
  hash_first(idx, salt, salt_len, digest);

  for (int i = 0; i < 2016; i++) {
    MD5_CTX ctx;
    MD5Init(&ctx);
    MD5Update(&ctx, (unsigned char *)digest, MD5_DIGEST_STRING_LENGTH - 1);
    MD5End(&ctx, digest);
  }
}

static void set_idxinfo(struct idxinfo *const idxinfo, const char *hex) {
  enum {
    TRIPLET_SINGLE,
    TRIPLET_PAIR,
    TRIPLET_FOUND,
  } triplet_state = TRIPLET_SINGLE;
  char triplet_char = '\0';

  enum {
    QUINTUPLET_NONE,
    QUINTUPLET_SINGLE,
    QUINTUPLET_PAIR,
    QUINTUPLET_TRIPLE,
    QUINTUPLET_QUADRUPLE,
    QUINTUPLET_FOUND,
  } quintuplet_states[16];
  for (int i = 0; i < 16; i++) {
    quintuplet_states[i] = QUINTUPLET_NONE;
  }

  while (*hex != '\0') {
    char c = *hex;

    if (triplet_state != TRIPLET_FOUND) {
      if (c == triplet_char) {
        triplet_state++;
      } else {
        triplet_char = c;
        triplet_state = TRIPLET_SINGLE;
      }
    }

    unsigned j = hexidx(c);
    for (unsigned i = 0; i < 16; i++) {
      if (quintuplet_states[i] != QUINTUPLET_FOUND) {
        if (i == j) {
          quintuplet_states[i]++;
        } else {
          quintuplet_states[i] = QUINTUPLET_NONE;
        }
      }
    }

    hex++;
  }

  if (triplet_state == TRIPLET_FOUND) {
    idxinfo->first_triple = triplet_char;
  } else {
    idxinfo->first_triple = '\0';
  }

  for (int i = 0; i < 16; i++) {
    idxinfo->has_quintuple[i] = quintuplet_states[i] == QUINTUPLET_FOUND;
  }
}

static bool iskey(const struct idxinfo *const candidate,
                  const struct idxinfo *const searchspace) {
  char c = candidate->first_triple;
  if (c == '\0') {
    return false;
  }

  unsigned j = hexidx(c);
  for (size_t i = 0; i < HASHES; i++) {
    if (searchspace[i].has_quintuple[j]) {
      return true;
    }
  }
  return false;
}

static void solution(const char *const input, char *const output,
                     void (*hash)(size_t, const unsigned char *, size_t,
                                  char *)) {
  char digest[MD5_DIGEST_STRING_LENGTH];

  const unsigned char *salt = (const unsigned char *)input;
  size_t salt_len = strlen(input) - 1;

  static struct idxinfoqueue hashinfo;
  for (int i = 0; i < 1000; i++) {
    hash(i, salt, salt_len, digest);
    set_idxinfo(hashinfo.elements + i, digest);
  }

  size_t i;
  unsigned count = 0;
  for (i = 0; count < 64; i++) {
    struct idxinfo info = hashinfo.elements[hashinfo.first];

    hash(i + HASHES, salt, salt_len, digest);
    set_idxinfo(hashinfo.elements + hashinfo.first, digest);
    hashinfo.first = (hashinfo.first + 1) % HASHES;

    if (iskey(&info, hashinfo.elements)) {
      DBG("%lu is key (%u/64)", i, count);
      count++;
    }
  }

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", i - 1);
}

static void solution1(const char *const input, char *const output) {
  solution(input, output, hash_first);
}

static void solution2(const char *const input, char *const output) {
  solution(input, output, hash_second);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
