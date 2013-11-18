#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define PREFIX_LENGTH 96

struct Packet {
  uint32_t seq_num;
  uint32_t ack_num;
  uint16_t data_len;
  bool ACK;
  bool FIN;
  char data[100];
};

#endif
