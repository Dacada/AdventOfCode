#include "intcode.h"
#include <aoclib.h>
#include <stdio.h>

#define NMACHINES 50
#define QUEUESIZE (1 << 12)

struct packet {
  long recipient;
  long x, y;
};

struct queue {
  size_t start;
  size_t end;
  struct packet elements[QUEUESIZE];
};

static void queue_init(struct queue *q) {
  q->start = 0;
  q->end = 0;
}

static void queue_enqueue(struct queue *q, struct packet p) {
  q->elements[q->end] = p;
  q->end = (q->end + 1) % QUEUESIZE;
  ASSERT(q->end != q->start, "queue got filled up!");
}

static struct packet queue_dequeue(struct queue *q) {
  ASSERT(q->end != q->start, "dequeuing empty queue!");
  struct packet p = q->elements[q->start];
  q->start = (q->start + 1) % QUEUESIZE;
  return p;
}

static bool queue_empty(struct queue *q) { return q->start == q->end; }

static struct packet get_packet(struct IntCodeMachine *machine) {
  struct packet packet;
  ASSERT(machine_recv_output(machine, &packet.recipient), "expected to receive output");
  machine_run(machine);
  ASSERT(machine_recv_output(machine, &packet.x), "expected to receive output");
  machine_run(machine);
  ASSERT(machine_recv_output(machine, &packet.y), "expected to receive output");
  machine_run(machine);
  return packet;
}

static void give_packet(struct IntCodeMachine *machine, struct packet packet) {
  ASSERT(machine_send_input(machine, packet.x), "expected to send input");
  machine_run(machine);
  ASSERT(machine_send_input(machine, packet.y), "expected to send input");
  machine_run(machine);
}

static void solution(const char *const input, char *const output, bool enable_nat) {
  struct IntCodeMachine original;
  machine_init(&original, input);
  machine_run(&original);

  struct IntCodeMachine machines[NMACHINES];
  struct queue queues[NMACHINES];
  for (long i = 0; i < NMACHINES; i++) {
    struct IntCodeMachine *machine = machines + i;
    machine_clone(machine, &original);
    ASSERT(machine_send_input(machine, i), "could not send address to machine %ld", i);
    machine_run(machine);

    queue_init(queues + i);
  }
  machine_free(&original);

  struct packet nat_memory = {.x = 0, .y = 0};
  int idle_count = 0;

  long result = 0;
  for (;;) {
    // Receive packets from all machines
    for (int i = 0; i < NMACHINES; i++) {
      struct IntCodeMachine *machine = machines + i;
      while (machine->has_output) {
        struct packet packet = get_packet(machine);
        if (packet.recipient == 255) {
          if (enable_nat) {
            nat_memory = packet;
          } else {
            result = packet.y;
            goto end;
          }
        } else {
          queue_enqueue(queues + packet.recipient, packet);
          DBG("Machine %d sends packet (%ld,%ld) to machine %ld", i, packet.x, packet.y, packet.recipient);
        }
      }
    }

    // Send packets to all machines
    bool idle = true;
    for (int i = 0; i < NMACHINES; i++) {
      struct IntCodeMachine *machine = machines + i;
      struct queue *queue = queues + i;

      if (queue_empty(queue)) {
        machine_send_input(machine, -1);
        machine_run(machine);
      } else {
        idle = false;
        struct packet packet = queue_dequeue(queue);
        give_packet(machine, packet);
        DBG("Machine %d received packet (%ld,%ld)", i, packet.x, packet.y);
      }
    }

    // Respond to idle condition
    if (enable_nat && idle) {
      idle_count++;
      if (idle_count >= 2) { // the network starts idle for whatever reason
        DBG("Network is idle. Sending (%ld,%ld) to address 0", nat_memory.x, nat_memory.y);

        if (nat_memory.y == result) {
          goto end;
        }
        result = nat_memory.y;

        give_packet(machines + 0, nat_memory);
      }
    } else {
      idle_count = 0;
    }
  }

end:
  snprintf(output, OUTPUT_BUFFER_SIZE, "%ld", result);
  for (int i = 0; i < NMACHINES; i++) {
    machine_free(machines + i);
  }
}

static void solution1(const char *const input, char *const output) { solution(input, output, false); }

static void solution2(const char *const input, char *const output) { solution(input, output, true); }

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
