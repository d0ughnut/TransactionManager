#include <iostream>
#include <memory>
#include <time.h>
#include <plog/Log.h>

#include "transaction_manager.h"

#include "result.h"
#include "api_store.h"
#include "packet_sender.h"
#include "state.h"
#include "config_accessor.h"

///// DEBUG_FLG
// #define WALLET_TEST
// #define DEBUG
// #define PURCHASE_TEST
// #define SELL_TEST
// #define IGNORE_TRANSCATION

TransactionManager::TransactionManager()
{
}

TransactionManager::~TransactionManager()
{
}

Result
TransactionManager::initialize()
{
  Result result;
  m_config = std::make_unique<ConfigAccessor>();
  result = load_param_from_config();
  if (result != Result::Success) goto _FAIL;

  m_api = std::make_unique<ApiStore>(m_config.get());
  if (!m_api) goto _FAIL;

  result = m_api->initialize();
  if (result != Result::Success) goto _FAIL;

  m_state = std::make_unique<StateManager>(m_config.get());
  if (!m_state) goto _FAIL;

  m_packet = std::make_unique<PacketSender>(m_config.get());
  result = connect_to_client();

  return Result::Success;

_FAIL:
  PLOG_FATAL << "Failed to initialize.";
  return Result::Failed;
}

Result
TransactionManager::load_param_from_config()
{
  Result result;
  result = m_config->load_file();
  if (result != Result::Success) goto _FAIL;

  m_src_currency = m_config->get_purchase_from_symbol();
  m_dst_currency = m_config->get_sell_to_symbol();

  if ((m_src_currency == "") || (m_dst_currency == "")) goto _FAIL;

  m_macd_s_param = m_config->get_macd_short_param();
  m_macd_l_param = m_config->get_macd_long_param();
  m_signal_param = m_config->get_signal_param();
  if ((m_macd_s_param < 0) || (m_macd_l_param < 0) || (m_signal_param < 0)) {
    goto _FAIL;
  }

  m_api_req_interval_sec = m_config->get_api_request_interval_sec();
  if (m_api_req_interval_sec < 0) goto _FAIL;

  m_symbol = m_src_currency + m_dst_currency;

  return Result::Success;

_FAIL:
  return Result::Failed;
}

Result
TransactionManager::connect_to_client()
{
  Result result;

  if (m_packet) {
      PLOG_INFO.printf("Connecting...");
      result = m_packet->con();
      if (result != Result::Success) {
        PLOG_WARNING << "Connection canceled.";
        m_packet = nullptr;
        goto _FAIL;
      }
      PLOG_INFO.printf("Connected to client.");
  }

  return Result::Success;

_FAIL:
  return Result::Failed;
}

void TransactionManager::exec() {
  struct timespec t = {m_api_req_interval_sec, 0};

  float server_timestamp;
  float macd_value, signal_value;

  Result result;

  // event loop
  while (1) {
    server_timestamp = m_api->get_server_unix_time();
    macd_value = m_api->get_macd(
                            m_symbol.c_str(),
                            server_timestamp,
                            m_macd_s_param,
                            m_macd_l_param
                        );
    PLOG_INFO.printf("Macd   (%02d, %02d, close): %f", m_macd_s_param, m_macd_l_param, macd_value);

    signal_value = m_api->get_macd_signal(
                              m_symbol.c_str(),
                              server_timestamp,
                              macd_value,
                              m_macd_s_param,
                              m_macd_l_param,
                              m_signal_param
                          );
    PLOG_INFO.printf("Signal (%02d)           : %f", m_signal_param, signal_value);

    if (m_packet) {
      PacketData data = {};
      data.macd = macd_value;
      data.signal = signal_value;
      m_packet->send_packet(&data);
    }

    bool should_purchase = macd_value > signal_value;
    StateManager::State next_state = m_state->get_next_state(should_purchase);

    float src_balance, dst_balance;
    double src_price, dst_price;
    double src_per_dst;
    switch (next_state) {
      case StateManager::State::BUY:
        PLOG_INFO << "Should be purchased.";
#ifndef IGNORE_TRANSCATION
        src_balance = m_api->get_balance(m_src_currency.c_str());
        dst_balance = m_api->get_balance(m_dst_currency.c_str());
        src_per_dst = m_api->get_price(m_symbol.c_str());

        PLOG_INFO.printf("src_balance: %f", src_balance);
        PLOG_INFO.printf("dst_balance: %f", dst_balance);

        PLOG_INFO.printf("transaction (purchase). src: %f, dst: %f", src_balance * src_per_dst, dst_balance);
        if ((src_balance * src_per_dst) > (dst_balance)) {
          PLOG_WARNING << "ignored transaction (purchase).";
          break;
        }

        PLOG_INFO.printf("Balance: %f", dst_balance);
        result = m_api->purchase(m_symbol.c_str(), dst_balance);
        if (result != Result::Success) goto _ABORT;
#endif
        break;
      case StateManager::State::SELL:
        PLOG_INFO << "Should be sell it.";
#ifndef IGNORE_TRANSCATION
        src_balance = m_api->get_balance(m_src_currency.c_str());
        dst_balance = m_api->get_balance(m_dst_currency.c_str());
        src_price = m_api->get_price(m_symbol.c_str());
        dst_price = m_api->get_price(m_symbol.c_str());

        PLOG_INFO.printf("transaction (sell). src: %f, dst: %f", src_balance * src_per_dst, dst_balance);
        if ((src_balance * src_per_dst) < (dst_balance)) {
          PLOG_WARNING << "ignored transaction (sell).";
          break;
        }

        PLOG_INFO.printf("Balance: %f", src_balance);
        result = m_api->sell(m_symbol.c_str(), src_balance);
        if (result != Result::Success) goto _ABORT;
#endif
        break;
      default:
        // idling.
        break;
    }

    nanosleep(&t, NULL);
  }

_ABORT:
  PLOG_FATAL << "Failed to exec.";
}
