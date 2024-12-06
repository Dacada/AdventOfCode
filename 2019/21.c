#include "intcode.h"
#include <aoclib.h>
#include <limits.h>
#include <stdio.h>

static void solution(const char *const input, char *const output, const char *const instructions) {
  struct IntCodeMachine machine;
  machine_init(&machine, input);

  machine_run(&machine);

#ifdef DEBUG
  char *prompt = machine_recv_output_string(&machine);
  fputs("> ", stderr);
  fputs(prompt, stderr);
  free(prompt);
#else
  machine_discard_output_string(&machine);
#endif

#ifdef DEBUG
  fputs(instructions, stderr);
#endif
  machine_send_input_string(&machine, instructions);

#ifdef DEBUG
  char *killcam = machine_recv_output_string(&machine);
  fputs("> ", stderr);
  fputs(killcam, stderr);
  free(killcam);

  if (machine.has_output) {
    snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", machine.output);
  } else {
    snprintf(output, OUTPUT_BUFFER_SIZE, "FAILURE");
  }
#else
  machine_discard_output_string(&machine);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", machine.output);
#endif

  machine_free(&machine);
}

static void solution1(const char *const input, char *const output) {
  /*
   *  A | B | C | D || J
   * --------------------
   *  0 | 0 | 0 | 0 || 1   // A=0 => We're about to fall anyway, so always jump
   *  0 | 0 | 0 | 1 || 1
   *  0 | 0 | 1 | 0 || 1
   *  0 | 0 | 1 | 1 || 1
   *  0 | 1 | 0 | 0 || 1
   *  0 | 1 | 0 | 1 || 1
   *  0 | 1 | 1 | 0 || 1
   *  0 | 1 | 1 | 1 || 1
   *  1 | 0 | 0 | 0 || 0   // D=0 => If we jump, we will land into a hole
   *  1 | 0 | 0 | 1 || 1   // In any other case, preemptively jump
   *  1 | 0 | 1 | 0 || 0
   *  1 | 0 | 1 | 1 || 1
   *  1 | 1 | 0 | 0 || 0
   *  1 | 1 | 0 | 1 || 1
   *  1 | 1 | 1 | 0 || 0
   *  1 | 1 | 1 | 1 || 0   // There is no need to jump if there's no holes
   *
   * J = ~A | (D & ~(B & C))
   */

  const char *const instructions = "OR C J\n"  // J = C | 0 = C
                                   "AND B J\n" // J = B & C
                                   "NOT J J\n" // J = ~(B & C)
                                   "AND D J\n" // J = D & ~(B & C)
                                   "NOT A T\n" // T = ~A
                                   "OR T J\n"  // J = T | (D & ~(B & C)) = ~A | (D & ~(B & C))
                                   "WALK\n";

  solution(input, output, instructions);
}

static void solution2(const char *const input, char *const output) {
  /*
   * No truth table now, 512 is a very large number :c
   *
   * We basically gotta add one thing to the expression of J: If
   * we were going to jump before but the tile we'll land on is
   * a hole, don't jump
   *
   * We continue to always jump if A is a hole, ignoring this.
   *
   * We continue to never jump if D is a hole, regardless of
   * this.
   *
   * We continue to never jump if B and C are neither holes
   * because there's still time to react.
   *
   * Therefore we need to replace only the ~(B & C) part.
   *
   * We will replace it by ~(B & C) & ...
   *
   * Running previous code to see how it does...
   *
   * Apparently it falls into H after two jumps. Ok. Adding H to
   * that AND.
   *
   * ok turns out that by adding H it just werks
   * wtf
   */

  const char *const instructions = "OR C J\n"  // J = C | 0 = C
                                   "AND B J\n" // J = B & C
                                   "NOT J J\n" // J = ~(B & C)
                                   "AND D J\n" // J = D & ~(B & C)

                                   "AND H J\n" // wtf

                                   "NOT A T\n" // T = ~A
                                   "OR T J\n"  // J = T | (D & ~(B & C)) = ~A | (D & ~(B & C))
                                   "RUN\n";

  solution(input, output, instructions);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
