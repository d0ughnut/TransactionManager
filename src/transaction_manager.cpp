#include <time.h>
#include <plog/Log.h>

#include <iostream>
#include <fstream>
#include <memory>
#include <future>
#include <thread>
#include <ctime>

#include "transaction_manager.h"
#include "result.h"
#include "api_store.h"
#include "packet_sender.h"
#include "state.h"
#include "config_accessor.h"
#include "constant.h"

#include "utils.h"

///// DEBUG_FLG
// #define IGNORE_TRANSACTION

const char* TransactionManager::LOG_FILE_DIR_PATH = "/var/log/transaction_manager/";
const char* TransactionManager::LOG_FILE_NAME     = "transaction.log";

TransactionManager::TransactionManager() {}

TransactionManager::~TransactionManager() {}

Result
TransactionManager::initialize() {
  Result result;
  // config 読込
  m_config = std::make_unique<ConfigAccessor>();
  result = load_param_from_config();
  if (result != Result::Success) {
    PLOG_ERROR << "failed to build config.";
    goto _FAIL;
  }

  // api wrappeer 初期化
  m_api = std::make_unique<ApiStore>(m_config.get());
  if (!m_api) {
    PLOG_ERROR << "failed to build api store.";
    goto _FAIL;
  }

  result = m_api->initialize();
  if (result != Result::Success) {
    PLOG_ERROR << "failed to init api store.";
    goto _FAIL;
  }

  // 状態管理
  m_state = std::make_unique<StateManager>(m_config.get(), this);
  if (!m_state) goto _FAIL;

  // クライアント接続 (失敗しても無視)
  m_packet = std::make_unique<PacketSender>(m_config.get());
  result = connect_to_client();

  return Result::Success;

_FAIL:
  PLOG_FATAL << "Failed to initialize.";
  return Result::Failed;
}

Result
TransactionManager::load_param_from_config() {
  Result result;
  result = m_config->load_file();
  if (result != Result::Success) goto _FAIL;

  m_src_currency = m_config->get_purchase_from_symbol();
  m_dst_currency = m_config->get_sell_to_symbol();

  if ((m_src_currency == "") || (m_dst_currency == "")) goto _FAIL;

  m_macd_s_param = m_config->get_macd_short_param();
  m_macd_l_param = m_config->get_macd_long_param();
  m_signal_param = m_config->get_signal_param();
  if ((m_macd_s_param <= 0) || (m_macd_l_param <= 0) || (m_signal_param <= 0)) {
    goto _FAIL;
  }

  m_cci_len = m_config->get_cci_length();
  m_tcci_len = m_config->get_tcci_length();
  if ((m_cci_len <= 0) || (m_tcci_len <= 0)) {
    goto _FAIL;
  }

  m_api_req_interval_sec = m_config->get_api_request_interval_sec();
  if (m_api_req_interval_sec < 0) goto _FAIL;

  // e.g. "BTCUSDT"
  m_symbol = m_src_currency + m_dst_currency;

  return Result::Success;

_FAIL:
  return Result::Failed;
}

Result
TransactionManager::connect_to_client() {
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

Result
TransactionManager::purchase() {
  double src_balance, dst_balance;
  double src_per_dst;
  PLOG_INFO << "Should be purchased.";
#ifndef IGNORE_TRANSACTION
  // 対象の通貨の残高をウォレットから取得
  src_balance = m_api->get_balance(m_src_currency.c_str());
  dst_balance = m_api->get_balance(m_dst_currency.c_str());
  // 現在の価格を取得
  src_per_dst = m_api->get_price(m_symbol.c_str());

  PLOG_INFO.printf("src_balance: %f", src_balance);
  PLOG_INFO.printf("dst_balance: %f", dst_balance);

  PLOG_INFO.printf("transaction (purchase). src: %f, dst: %f", src_balance * src_per_dst, dst_balance);
  // 買えない、買う必要がない場合は取引機会を無視
  if ((src_balance * src_per_dst) > (dst_balance)) {
    PLOG_WARNING << "ignored transaction (purchase).";
    return Result::Success;
  }

  PLOG_INFO.printf("Balance: %f", dst_balance);

  log_transaction(TransactionSignal::PURCHASE);

  return m_api->purchase(m_symbol.c_str(), dst_balance);
#else
  return Result::Success;
#endif
}

Result
TransactionManager::sell() {
  double src_balance, dst_balance;
  double src_per_dst;
  PLOG_INFO << "Should be sell it.";
#ifndef IGNORE_TRANSACTION
  // 対象の通貨の残高をウォレットから取得
  src_balance = m_api->get_balance(m_src_currency.c_str());
  dst_balance = m_api->get_balance(m_dst_currency.c_str());
  // 現在の価格を取得
  src_per_dst = m_api->get_price(m_symbol.c_str());

  PLOG_INFO.printf("transaction (sell). src: %f, dst: %f", src_balance * src_per_dst, dst_balance);
  // 売れない、売る必要がない場合は取引機会を無視
  if ((src_balance * src_per_dst) < (dst_balance)) {
    PLOG_WARNING << "ignored transaction (sell).";
    return Result::Success;
  }

  PLOG_INFO.printf("Balance: %f", src_balance);

  log_transaction(TransactionSignal::SELL);

  return m_api->sell(m_symbol.c_str(), src_balance);
#else
  return Result::Success;
#endif
}

TransactionSignal
TransactionManager::request_macd(Long time, PacketData* data) {
  double macd_value, signal_value;

  macd_value = m_api->get_macd(
      m_symbol.c_str(),
      time,
      m_macd_s_param,
      m_macd_l_param
  );

  signal_value = m_api->get_macd_signal(
      m_symbol.c_str(),
      time,
      macd_value,
      m_macd_s_param,
      m_macd_l_param,
      m_signal_param
  );

  PLOG_INFO.printf("Macd   (%02d, %02d, close): %f",
      m_macd_s_param,
      m_macd_l_param,
      macd_value
  );
  PLOG_INFO.printf("Signal (%02d)           : %f",
      m_signal_param,
      signal_value
  );

  if (data) {
    data->macd   = macd_value;
    data->signal = signal_value;
  }

  if (macd_value > signal_value) {
    return TransactionSignal::PURCHASE;
  } else {
    return TransactionSignal::SELL;
  }
}

TransactionSignal
TransactionManager::request_cci(Long time, PacketData* sig) {
  // TODO: 閾値 (変更可能にする)
  const int high_th = 100;
  const int low_th  = -100;
  const int d_high_th = 250;
  const int d_low_th  = -250;

  // 前回リクエスト時の値
  static double prev_cci = 0;
  static TransactionSignal last_sig = TransactionSignal::IDLE;

  double cur_cci = m_api->get_cci(
      m_symbol.c_str(),
      m_cci_len,
      time
  );

  double cur_tcci = m_api->get_cci(
      m_symbol.c_str(),
      m_tcci_len,
      time
  );

  PLOG_INFO.printf(" Cci    (%02d)           : %f",
      m_cci_len,
      cur_cci
  );

  PLOG_INFO.printf(" Tcci   (%02d)           : %f",
      m_tcci_len,
      cur_tcci
  );

  TransactionSignal signal = TransactionSignal::IDLE;
#if 0
  if ((prev_cci > high_th) && (cur_cci <= high_th)) {
    // 前回上閾値以上、今回上閾値以下の場合 (= 売りシグナル)
    PLOG_INFO << "cci signal (sell)";
    signal = TransactionSignal::SELL;
    goto _OUT;
  } else if ((prev_cci < low_th) && (cur_cci >= low_th)) {
    // 前回下閾値以下、今回下閾値以上の場合 (= 買いシグナル)
    PLOG_INFO << "cci signal (purchase)";
    signal = TransactionSignal::PURCHASE;
    goto _OUT;
  } else if ((prev_cci > d_high_th) && (cur_cci <= d_high_th)) {
    // 前回上閾値以上、今回上閾値以下の場合 (= 強い売りシグナル)
    PLOG_INFO << "cci signal (f_sell)";
    signal = TransactionSignal::F_SELL;
    goto _OUT;
  } else if ((prev_cci < d_low_th) && (cur_cci >= d_low_th)) {
    // 前回下閾値以下、今回下閾値以上の場合 (= 強い買いシグナル)
    PLOG_INFO << "cci signal (f_purchase)";
    signal = TransactionSignal::F_PURCHASE;
    goto _OUT;
  }
#endif

  if ((cur_cci > 0) && (cur_tcci > 0)) {
    PLOG_INFO << "cci signal (w_purchase)";
    signal = TransactionSignal::PURCHASE;
  } else if ((cur_cci <= 0) && (cur_tcci <= 0)) {
    PLOG_INFO << "cci signal (w_sell)";
    signal = TransactionSignal::SELL;
  }

_OUT:
  prev_cci = cur_cci;

  if (signal == last_sig) {
    last_sig = signal;
    return TransactionSignal::IDLE;
  } else {
    last_sig = signal;
    return signal;
  }
}

void
TransactionManager::entry(Property::State init_state) {
  m_state->entry(init_state);
  m_state->exec(
      TransactionSignal::IDLE,
      TransactionSignal::IDLE
  );
}

void
TransactionManager::exec() {
  struct timespec t = {m_api_req_interval_sec, 0};

  double server_timestamp;

  // イベントループ
  // TODO: SIGINT 以外で正常に抜けられるようにする
  while (1) {
    server_timestamp = m_api->get_server_unix_time();

    PacketData* data = nullptr;
    if (m_packet) {
      data = new PacketData();
    }

    // do in worker th.
    auto cci_th = std::async(
        std::launch::async,
        [&] { return request_cci(server_timestamp, data); }
    );

    // do in main th.
    TransactionSignal macd_sig = request_macd(server_timestamp, data);
    // join & get cci
    TransactionSignal cci_sig = cci_th.get();

    // クライアントと接続済みならパケットを送信
    if (data) {
      m_packet->send_packet(data);
      delete data;
    }

    Result ret = m_state->exec(macd_sig, cci_sig);
    if (ret != Result::Success) {
      PLOG_ERROR << "Failed to manage state.";
      return;
    }


    // wait (過剰にリクエストしない為)
    nanosleep(&t, NULL);
  }
}

void
TransactionManager::log_transaction(TransactionSignal signal) {
  if (!FileUtils::is_exist(TransactionManager::LOG_FILE_DIR_PATH)) {
    FileUtils::make_directory(TransactionManager::LOG_FILE_DIR_PATH);
  }

  std::string path = std::string(TransactionManager::LOG_FILE_DIR_PATH) + std::string(TransactionManager::LOG_FILE_NAME);
  std::ofstream ofs;

  std::time_t now = std::time(nullptr);

  ofs.open(path, std::ios_base::app);

  std::string str_signal;

  if (signal == TransactionSignal::PURCHASE) {
    str_signal = std::string("PURCHASE");
  } else if (signal == TransactionSignal::SELL) {
    str_signal = std::string("SELL    ");
  } else {
    str_signal = std::string("UNKNOWN ");
  }

  ofs << str_signal << ": " << std::ctime(&now);
}
