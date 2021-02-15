#include <errno.h>
#include <plog/Log.h>

#include <cstring>

#include "packet_sender.h"

PacketSender::PacketSender(ConfigAccessor* config)
    : m_addr(config->get_client_addr()),
      m_port(config->get_client_port()) {}

PacketSender::~PacketSender() {
  discon();
}

Result
PacketSender::con() {
  if ((m_addr == "") || (m_port < 0)) {
    PLOG_WARNING << "Client info is not set.";
    return Result::Failed;
  }

  PLOG_INFO.printf("Client addr: %s", m_addr.c_str());
  PLOG_INFO.printf("Client port: %d", m_port);

  m_sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (m_sockfd < 0) {
    PLOG_WARNING << "Failed to build socket.";
    return Result::Failed;
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_port);
  addr.sin_addr.s_addr = inet_addr(m_addr.c_str());

  int ret = connect(
      m_sockfd,
      (struct sockaddr *) &addr,
      sizeof(struct sockaddr_in)
  );

  if (ret < 0) {
    PLOG_ERROR.printf("failed to connect: %s", strerror(errno));
    return Result::Failed;
  }

  return Result::Success;
}

Result
PacketSender::send_packet(PacketData* data) {
  send(m_sockfd, data, sizeof(PacketData), 0);
  return Result::Success;
}

void
PacketSender::discon() {
  if (m_sockfd > 0) {
    close(m_sockfd);
  }
}
