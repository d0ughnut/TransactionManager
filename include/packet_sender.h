#pragma once

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <iostream>

#include "config_accessor.h"
#include "result.h"

typedef struct _PacketData {
  double macd;
  double signal;
} PacketData;

class PacketSender {
 private:
  int m_sockfd;

  std::string m_addr;
  int m_port;

 public:
  explicit PacketSender(ConfigAccessor* config);
  ~PacketSender();

  Result con();
  Result send_packet(PacketData *data);
  void discon();
};
