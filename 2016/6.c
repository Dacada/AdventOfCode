#include <aoclib.h>
#include <stdio.h>

static void parse(const char *const input, size_t *const nmsgs, size_t *const msglen) {
  for (*msglen = 0;; *msglen += 1) {
    if (input[*msglen] == '\n') {
      break;
    }
  }
  for (*nmsgs = 0;; *nmsgs += 1) {
    if (input[*nmsgs * (*msglen + 1)] == '\0') {
      break;
    }
  }
}

static void solution(const char *const input, char *const output, bool less_likely) {
  size_t nmsgs;
  size_t msglen;
  parse(input, &nmsgs, &msglen);

  DBG("%lu %lu", nmsgs, msglen);

  char final_message[msglen + 1];
  for (size_t letter = 0; letter < msglen; letter++) {
    unsigned letter_count['z' - 'a' + 1] = {0};
    for (size_t message = 0; message < nmsgs; message++) {
      size_t idx = message * (msglen + 1) + letter;
      char c = input[idx];
      DBG("'%c'(%d) at %lu", c, c, idx);
      letter_count[c - 'a']++;
    }

    char bestl = '\0';
    unsigned bestl_count = 0;
    for (char l = 'a'; l <= 'z'; l++) {
      unsigned count = letter_count[l - 'a'];

      bool update;
      if (less_likely) {
        update = count > 0 && (bestl_count == 0 || count < bestl_count);
      } else {
        update = count > bestl_count;
      }

      if (update) {
        bestl = l;
        DBG("%c, %u", l, count);
        bestl_count = count;
      }
    }

    final_message[letter] = bestl;
  }
  final_message[msglen] = '\0';

  snprintf(output, OUTPUT_BUFFER_SIZE, "%s", final_message);
}

static void solution1(const char *const input, char *const output) { solution(input, output, false); }

static void solution2(const char *const input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
