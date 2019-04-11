#include <aoclib.h>
#include <bsd/md5.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

static void solution(const char *const input, char *const output, const uint8_t digest2_val) {
  size_t len = strlen(input);
  MD5_CTX base_ctx, iter_ctx;
  MD5Init(&base_ctx);
  MD5Update(&base_ctx, (const uint8_t *const)input, len);

  for (unsigned long result = 0; result < ULONG_MAX; result++) {
    memcpy(&iter_ctx, &base_ctx, sizeof(MD5_CTX));
    
    sprintf(output, "%lu", result);
    len = strlen(output);
    MD5Update(&iter_ctx, (uint8_t*)output, len);
    
    uint8_t digest[MD5_DIGEST_LENGTH];
    MD5Final(digest, &iter_ctx);
    if (digest[0] == 0 && digest[1] == 0 && digest[2] <= digest2_val)
      break;
  }
}

static void solution1(const char *const input, char *const output) {
  solution(input, output, 0xf);
}

static void solution2(const char *const input, char *const output) {
  solution(input, output, 0);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
