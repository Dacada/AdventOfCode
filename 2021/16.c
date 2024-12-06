#include <aoclib.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>

struct packet {
  unsigned version;
  enum {
    PACKET_LITERAL,
    PACKET_OPERATOR,
    PACKET_SUM,
    PACKET_PROD,
    PACKET_MIN,
    PACKET_MAX,
    PACKET_GT,
    PACKET_LT,
    PACKET_EQ,
  } type;
};

struct literalPacket {
  struct packet header;
  unsigned long number;
};

struct operationPacket {
  struct packet header;
  size_t length;
  struct packet **subpackets;
};

static unsigned unhex(const char **const hex) {
  ASSERT(isxdigit(**hex), "parse error");

  unsigned ret;
  if (isdigit(**hex)) {
    ret = **hex - '0';
  } else {
    ret = **hex - 'A' + 10;
  }
  *hex += 1;

  return ret;
}

static unsigned extract_bits(const char **const hex, unsigned *const reserve, unsigned *const reserve_len,
                             unsigned count) {
  // DBG("START EXTRACT BITS, reserve=0x%X, len=%u, count=%u", *reserve, *reserve_len, count);
  unsigned ret = 0;
  if (count >= *reserve_len) {
    // DBG("%u >= %u", count, *reserve_len);

    ret = *reserve;
    count -= *reserve_len;

    // DBG("set ret to reserve (0x%X)", *reserve);
    // DBG("count -= %u (%u)", *reserve_len, count);

    *reserve = 0;
    *reserve_len = 0;

    while (count >= 4) {
      // DBG("count >= 4 (%u), unhex", count);
      ret <<= 4;
      ret |= unhex(hex);
      count -= 4;
    }

    if (count == 0) {
      // DBG("count == 0");
      // DBG("END EXTRACT BITS, EXTRACTED 0x%X\n", ret);
      return ret;
    }

    *reserve = unhex(hex);
    *reserve_len = 4;
    // DBG("unhex to get some reserve (0x%X)", *reserve);
  }

  unsigned mask = 0;
  for (unsigned i = 0; i < count; i++) {
    mask <<= 1;
    mask |= 1;
  }
  // DBG("create mask from count=%u -> 0x%X", count, mask);

  mask <<= *reserve_len - count;
  // DBG("shift mask by %u into 0x%X", *reserve_len - count, mask);

  ret <<= count;
  ret |= (*reserve & mask) >> (*reserve_len - count);
  // DBG("or *reserve & mask (0x%X) into ret: 0x%X", (*reserve & mask) >> (*reserve_len - count), ret);

  *reserve &= ~mask;
  *reserve_len -= count;
  // DBG("and reserve with ~mask (0x%X), becomes 0x%X", ~mask, *reserve);
  // DBG("subtract count from reserve_len, becomes %u", *reserve_len);

  // DBG("END EXTRACT BITS, EXTRACTED 0x%X\n", ret);
  return ret;
}

static struct literalPacket *parse_literalPacket(const char **const input, unsigned *const bits, unsigned *const nbits,
                                                 unsigned *const bitsConsumed) {
  struct literalPacket *res = malloc(sizeof(*res));
  res->number = 0;

  const unsigned max_nibbles = sizeof(res->number) * 8 / 4;
  unsigned nibbles = 0;

  bool cont = true;
  while (cont) {
    res->number <<= 4;
    cont = extract_bits(input, bits, nbits, 1);
    char nibble = extract_bits(input, bits, nbits, 4);
    if (bitsConsumed != NULL) {
      *bitsConsumed += 5;
    }
    res->number |= nibble;
    nibbles++;
    ASSERT(nibbles <= max_nibbles, "integer too big");
  }

  return res;
}

static struct packet *parse_packet(const char **, unsigned *, unsigned *, unsigned *);
static struct operationPacket *parse_operationPacket(const char **const input, unsigned *const bits,
                                                     unsigned *const nbits, unsigned *const bitsConsumed) {
  struct operationPacket *res = malloc(sizeof(*res));

  bool knowNumber = extract_bits(input, bits, nbits, 1);
  if (bitsConsumed != NULL) {
    *bitsConsumed += 1;
  }

  if (knowNumber) {
    res->length = extract_bits(input, bits, nbits, 11);
    if (bitsConsumed != NULL) {
      *bitsConsumed += 11;
    }

    res->subpackets = malloc(sizeof(res->subpackets) * res->length);
    for (size_t i = 0; i < res->length; i++) {
      res->subpackets[i] = parse_packet(input, bits, nbits, bitsConsumed);
    }
  } else {
    unsigned sub_bitsToConsume = extract_bits(input, bits, nbits, 15);
    if (bitsConsumed != NULL) {
      *bitsConsumed += 15;
    }
    unsigned sub_bitsConsumed = 0;

    res->length = 0;
    size_t size = 4;
    res->subpackets = malloc(size * sizeof(*res->subpackets));

    while (sub_bitsConsumed < sub_bitsToConsume) {
      if (res->length >= size) {
        size *= 2;
        res->subpackets = realloc(res->subpackets, size * sizeof(*res->subpackets));
      }

      res->subpackets[res->length] = parse_packet(input, bits, nbits, &sub_bitsConsumed);
      res->length += 1;
    }

    ASSERT(sub_bitsConsumed == sub_bitsToConsume, "parse error");
    if (bitsConsumed != NULL) {
      *bitsConsumed += sub_bitsConsumed;
    }
  }

  return res;
}

static struct packet *parse_packet(const char **const input, unsigned *const bits, unsigned *const nbits,
                                   unsigned *const bitsConsumed) {
  unsigned version = extract_bits(input, bits, nbits, 3);
  unsigned type = extract_bits(input, bits, nbits, 3);
  if (bitsConsumed != NULL) {
    *bitsConsumed += 6;
  }

  struct packet *res;
  if (type == 4) {
    res = (struct packet *)parse_literalPacket(input, bits, nbits, bitsConsumed);
    res->type = PACKET_LITERAL;
  } else {
    res = (struct packet *)parse_operationPacket(input, bits, nbits, bitsConsumed);
    switch (type) {
    case 0:
      res->type = PACKET_SUM;
      break;
    case 1:
      res->type = PACKET_PROD;
      break;
    case 2:
      res->type = PACKET_MIN;
      break;
    case 3:
      res->type = PACKET_MAX;
      break;
    case 5:
      res->type = PACKET_GT;
      break;
    case 6:
      res->type = PACKET_LT;
      break;
    case 7:
      res->type = PACKET_EQ;
      break;
    default:
      FAIL("invalid packet type");
    }
  }
  res->version = version;

  return res;
}

static struct packet *parse_input(const char *input) {
  unsigned remainingBits = 0;
  unsigned remainingBitsCount = 0;
  struct packet *packets = parse_packet(&input, &remainingBits, &remainingBitsCount, NULL);
  ASSERT(remainingBits == 0 || remainingBitsCount == 0, "parse error");
  while (*input == '0') {
    input += 1;
  }
  ASSERT(*input == '\0' || *input == '\n', "parse error %s", input);
  return packets;
}

__attribute__((pure)) static unsigned add_up_versions(struct packet *packet) {
  unsigned sum = packet->version;

  if (packet->type >= PACKET_OPERATOR) {
    struct operationPacket *operationPacket = (struct operationPacket *)packet;
    for (size_t i = 0; i < operationPacket->length; i++) {
      sum += add_up_versions(operationPacket->subpackets[i]);
    }
  }

  return sum;
}

static void free_packet(struct packet *packet) {
  if (packet->type >= PACKET_OPERATOR) {
    struct operationPacket *operationPacket = (struct operationPacket *)packet;
    for (size_t i = 0; i < operationPacket->length; i++) {
      free_packet(operationPacket->subpackets[i]);
    }
    free(operationPacket->subpackets);
  }
  free(packet);
}

static void solution1(const char *const input, char *const output) {
  struct packet *packets = parse_input(input);

  unsigned result = add_up_versions(packets);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%u", result);
  free_packet(packets);
}

__attribute__((pure)) static unsigned long eval(struct packet *packet) {
  if (packet->type == PACKET_LITERAL) {
    return ((struct literalPacket *)packet)->number;
  }

  struct operationPacket *p = (struct operationPacket *)packet;

  switch (packet->type) {
  case PACKET_SUM: {
    unsigned long sum = 0;
    for (size_t i = 0; i < p->length; i++) {
      sum += eval(p->subpackets[i]);
    }
    DBG("%lu", sum);
    return sum;
  } break;
  case PACKET_PROD: {
    unsigned long prod = 1;
    for (size_t i = 0; i < p->length; i++) {
      prod *= eval(p->subpackets[i]);
    }
    DBG("%lu", prod);
    return prod;
  } break;
  case PACKET_MIN: {
    unsigned long min = ULONG_MAX;
    for (size_t i = 0; i < p->length; i++) {
      unsigned long n = eval(p->subpackets[i]);
      if (n < min) {
        min = n;
      }
    }
    DBG("%lu", min);
    return min;
  } break;
  case PACKET_MAX: {
    unsigned long max = 0;
    for (size_t i = 0; i < p->length; i++) {
      unsigned long n = eval(p->subpackets[i]);
      if (n > max) {
        max = n;
      }
    }
    DBG("%lu", max);
    return max;
  } break;
  case PACKET_GT:
    ASSERT(p->length == 2, "invalid packet length");
    return eval(p->subpackets[0]) > eval(p->subpackets[1]);
    break;
  case PACKET_LT:
    ASSERT(p->length == 2, "invalid packet length");
    return eval(p->subpackets[0]) < eval(p->subpackets[1]);
    break;
  case PACKET_EQ:
    ASSERT(p->length == 2, "invalid packet length");
    return eval(p->subpackets[0]) == eval(p->subpackets[1]);
    break;
  case PACKET_LITERAL:
  case PACKET_OPERATOR:
  default:
    FAIL("invalid packet type");
  }
}

static void solution2(const char *const input, char *const output) {
  struct packet *packets = parse_input(input);

  unsigned long result = eval(packets);

  snprintf(output, OUTPUT_BUFFER_SIZE, "%lu", result);
  free_packet(packets);
}

int main(int argc, char *argv[]) { return aoc_run(argc, argv, solution1, solution2); }
