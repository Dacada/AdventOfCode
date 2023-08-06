#include <aoclib.h>
#include <stdio.h>
#include <stdbool.h>

#define NSECONDS 2503
#define NREINDEERS 9

#define ISNUM(n) ((n) >= '0' && (n) <= '9')

static unsigned int calculate_distance(const unsigned int speed, const unsigned int run_time, const unsigned int stop_time) {
  unsigned int total_time = run_time + stop_time;
  unsigned int full_iterations = NSECONDS / total_time;
  unsigned int seconds_last_iteration = NSECONDS % total_time;

  unsigned int total_seconds_running = full_iterations * run_time;
  if (seconds_last_iteration > run_time)
    total_seconds_running += run_time;
  else
    total_seconds_running += seconds_last_iteration;

  return total_seconds_running * speed;
}

static unsigned int parse_number(const char *const input, unsigned int *const i) {
  unsigned int j = *i;
  
  while (!ISNUM(input[j])) j++;

  unsigned int number = 0;
  while (ISNUM(input[j])) {
    number = number*10 + input[j]-0x30;
    j++;
  }

  *i = j;
  return number;
}

static void parse_until_eol(const char *const input, unsigned int *const i) {
  unsigned int j = *i;
  while (input[j] != '\n') j++;
  *i = j;
}

static unsigned int parse_line1(const char *const input, unsigned int *const i) {
  const unsigned int speed = parse_number(input, i);
  const unsigned int run_time = parse_number(input, i);
  const unsigned int stop_time = parse_number(input, i);
  parse_until_eol(input, i);
  
  return calculate_distance(speed, run_time, stop_time);
}

__attribute__((pure))
static unsigned int parse1(const char *const input) {
  unsigned int max = 0;
  
  for (unsigned int i=0;; i++) {
    if (input[i] == '\0') {
      break;
    }

    unsigned int current = parse_line1(input, &i);
    ASSERT(input[i] == '\n', "Did not parse full line");

    if (current > max) {
      max = current;
    }
  }

  return max;
}

static void solution1(const char *const input, char *const output) {
  unsigned int distance = parse1(input);
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", distance);
}

struct reindeer {
  unsigned int speed;
  unsigned int run_time;
  unsigned int stop_time;
  
  unsigned int distance;
  unsigned int points;
  unsigned int running_counter;
  unsigned int stopped_counter;
  bool is_running;
};

static void parse_line2(const char *const input, unsigned int *const i, struct reindeer *const rndr) {
  unsigned int speed = parse_number(input, i);
  unsigned int run_time = parse_number(input, i);
  unsigned int stop_time = parse_number(input, i);
  parse_until_eol(input, i);

  rndr->speed = speed;
  rndr->run_time = run_time;
  rndr->stop_time = stop_time;

  rndr->distance = 0;
  rndr->points = 0;
  rndr->running_counter = 0;
  rndr->stopped_counter = 0;
  rndr->is_running = true;
}

static void parse2(const char *const input, struct reindeer *const array) {
  unsigned int j = 0;
  for (unsigned int i=0;; i++) {
    if (input[i] == '\0') {
      break;
    }

    parse_line2(input, &i, array + j);
    j++;
    
    ASSERT(input[i] == '\n', "Did not parse full line");
  }
}

static void solution2(const char *const input, char *const output) {
  struct reindeer rdrs[NREINDEERS];
  parse2(input, rdrs);

  for (int i=0; i<NSECONDS; i++) {
    for (int j=0; j<NREINDEERS; j++) {
      if (rdrs[j].is_running) {
	rdrs[j].distance += rdrs[j].speed;
      }
      
      if (rdrs[j].is_running) {
	rdrs[j].running_counter++;
	if (rdrs[j].running_counter >= rdrs[j].run_time) {
	  rdrs[j].running_counter = 0;
	  rdrs[j].is_running = false;
	}
      } else {
	rdrs[j].stopped_counter++;
	if (rdrs[j].stopped_counter >= rdrs[j].stop_time) {
	  rdrs[j].stopped_counter = 0;
	  rdrs[j].is_running = true;
	}
      }
    }
    
    unsigned int head_distance = 0;
    for (int j=0; j<NREINDEERS; j++) {
      if (rdrs[j].distance > head_distance) {
	head_distance = rdrs[j].distance;
      }
    }
    
    for (int j=0; j<NREINDEERS; j++) {
      if (rdrs[j].distance == head_distance)
	rdrs[j].points++;
    }
  }

  unsigned int winner_points = 0;
  for (int i=0; i<NREINDEERS; i++) {
    if (rdrs[i].points > winner_points) {
      winner_points = rdrs[i].points;
    }
  }
  
  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", winner_points);
}

int main(int argc, char *argv[]) {
  return aoc_run(argc, argv, solution1, solution2);
}
