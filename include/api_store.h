#pragma once

#include <json/json.h>

#include <memory>
#include <string>
#include <vector>

#include "binance-cxx-api/binance.h"
#include "binance-cxx-api/binance_websocket.h"
#include "binance-cxx-api/binance_logger.h"


#include "result.h"
#include "config_accessor.h"
#include "alias.h"
#include "data.h"

using namespace binance;

class ApiStore {
 private:
  static const int API_CALL_LEFT;
  static const int RECV_WINDOW;
  static const std::string API_DIR;
  static const std::string API_KEY_PATH;
  static const std::string API_SECRET_PATH;

  std::string m_api_key;
  std::string m_api_secret;

  Server m_server;
  Market m_market;
  Account m_account;

  Result write_api_to_file(const std::string& file_path, const std::string& buffer);

 public:
  explicit ApiStore(ConfigAccessor* config);
  ~ApiStore();

  Result initialize();

  Result get_kline(
      Json::Value& result,
      const char* symbol,
      const char* interval_day,
      Long open_unix_time = 0,
      Long close_unix_time = 0,
      int limit = 500
  );

  Long get_server_unix_time();
  double get_macd_signal(const char* symbol, Long server_unix_time, double recent_macd, int s, int l, int range);
  double get_macd(const char* symbol, Long server_unix_time, int s, int l);
  double get_ema(const std::vector<double>& c_pricies, int range);
  double get_balance(const char* symbol);
  double get_price(const char* symbol);
  double get_cci(const char* symbol, int range, Long server_unix_time);
  std::vector<double> get_c_pricies_from_klines(Json::Value& buffer);
  std::vector<TradeData> get_trade_data_from_klines(Json::Value& buffer);
  std::vector<double> get_tp(Json::Value& buffer);

  Result purchase(const char* symbol, double balance);
  Result sell(const char* symbol, double balance);
};
