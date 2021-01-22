#pragma once

#include <memory>
#include <json/json.h>
#include <string>

#include "result.h"
#include "config_accessor.h"

#include "binance-cxx-api/binance.h"
#include "binance-cxx-api/binance_websocket.h"
#include "binance-cxx-api/binance_logger.h"

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
    ApiStore(ConfigAccessor* config);
    ~ApiStore();

    Result initialize();

    Result get_kline(
        Json::Value& result,
        const char* symbol,
        const char* interval_day,
        long open_unix_time = 0,
        long close_unix_time = 0,
        int limit = 500
    );

    float get_macd_signal(const char* symbol, long server_unix_time, float recent_macd, int s, int l, int range);
    float get_macd(const char* symbol, long server_unix_time, int s, int l);
    float get_ema(std::vector<float>& c_pricies, int range);
    std::vector<float> get_c_pricies_from_klines(Json::Value& result);

    long get_server_unix_time();

    float get_balance(const char* symbol);
    Result purchase(const char* symbol, float balance);
    Result sell(const char* symbol, float balance);

    // void print(Json::Value& json_result);
};
