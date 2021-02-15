#pragma once

#include <string>
#include <memory>

#include "result.h"
#include "config_accessor.h"
#include "api_store.h"
#include "packet_sender.h"
#include "state.h"
#include "signal.h"
#include "alias.h"
#include "constant.h"

class StateManager;

class TransactionManager {
 public:
  TransactionManager();
  ~TransactionManager();

  Result initialize();
  void entry(Property::State init_state);
  void exec();
  Result purchase();
  Result sell();

 private:
  std::unique_ptr<ConfigAccessor> m_config;
  std::unique_ptr<ApiStore> m_api;
  std::unique_ptr<StateManager> m_state;
  std::unique_ptr<PacketSender> m_packet;

  TransactionSignal request_macd(Long time, PacketData* data);
  TransactionSignal request_cci(Long time, PacketData* data);

  int m_macd_s_param, m_macd_l_param, m_signal_param;
  int m_cci_len;

  std::string m_src_currency, m_dst_currency;
  std::string m_symbol;
  int m_api_req_interval_sec;

  Result load_param_from_config();
  Result connect_to_client();
};
