#pragma once

#include <string>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "config_accessor.h"
#include "result.h"

typedef struct _PacketData {
  float macd;
  float signal;
} PacketData;

class PacketSender {
  private:
    int m_sockfd;

    std::string m_addr;
    int m_port;

  public:
    PacketSender(ConfigAccessor* config);
    ~PacketSender();

    Result con();
    Result send_packet(PacketData *data);
    void discon();
};
