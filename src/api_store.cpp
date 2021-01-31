#include <memory>
#include <iostream>
#include <json/json.h>
#include <ctime>
#include <typeinfo>
#include <plog/Log.h>
#include <fstream>

#include "api_store.h"
#include "utils.h"

const int ApiStore::API_CALL_LEFT = 5;
const int ApiStore::RECV_WINDOW = 10000;

const std::string ApiStore::API_DIR         = std::string(getenv("HOME")) + std::string("/.bitrader/");
const std::string ApiStore::API_KEY_PATH    = API_DIR + std::string("key");
const std::string ApiStore::API_SECRET_PATH = API_DIR + std::string("secret");

#define X2_LOGIC

ApiStore::ApiStore(ConfigAccessor* config) :
m_api_key(config->get_api_key()),
m_api_secret(config->get_api_secret()),
m_market(m_server),
m_account(m_server)
{
}

ApiStore::~ApiStore()
{
}

Result
ApiStore::initialize()
{
  Result result;

  if ((m_api_key == "") || (m_api_secret == "")) {
    PLOG_ERROR << "API key is not set.";
    return Result::Failed;
  }

  if (!FileUtils::is_exist(API_DIR)) {
    result = FileUtils::make_directory(API_DIR);
    if (result != Result::Success) return Result::Failed;
  }

  result = write_api_to_file(API_KEY_PATH, m_api_key);
  if (result != Result::Success) return Result::Failed;

  result = write_api_to_file(API_SECRET_PATH, m_api_secret);
  if (result != Result::Success) return Result::Failed;

  return Result::Success;
}

Result
ApiStore::write_api_to_file(const std::string& file_path, const std::string& buffer)
{
  std::ofstream ofs(file_path);
  if (!ofs) {
    PLOG_ERROR << "Failed to open file.";
    PLOG_WARNING << strerror(errno);
    return Result::Failed;
  }

  ofs << buffer;

  return Result::Success;
}

long
ApiStore::get_server_unix_time()
{
  Json::Value result;
  int retry = API_CALL_LEFT;

  while (1) {
    result.clear();
    binanceError_t ret = m_server.getTime(result);
    if (ret == binanceSuccess) {
      break;
    } else {
      --retry;
      PLOG_WARNING.printf("Failed to request. Try Again (Last: %d)", retry); 
      if (retry <= 0) {
        PLOG_ERROR << "Failed to get server_timestamp.";
        return -1;
      }
    }
  }

  std::string buffer = std::string(result["serverTime"].asString());
  return std::stol(buffer);
}

float
ApiStore::get_macd_signal(const char* symbol, long server_unix_time, float recent_macd, int s, int l, int range)
{
  std::string str_t = std::to_string(server_unix_time);
  str_t = str_t.substr(0, 10);
  long unix_t = std::stol(str_t);

  float num = 0.0f;

  std::vector<float> macds;
  // get_ema() に渡す vector が降順な為、逆から詰める
  for (int i = (range * 2) - 1; i > 0; --i) {
    std::tm *tm = std::localtime(&unix_t);
    tm->tm_hour -= 4 * i;
    std::time_t t = std::mktime(tm);

    // 落としたミリ秒分繰り上げる
    float m = get_macd(symbol, t * 1000, s, l);

    macds.push_back(m);
  }

  macds.push_back(recent_macd);

  return get_ema(macds, range);
}

float
ApiStore::get_macd(const char* symbol, long server_unix_time, int s_range, int l_range)
{
  // interval (足間) * th が 24 になるよう設定する
  const int th = 6;

  float s_ema, l_ema;
  Json::Value buffer;
  std::vector<float> c_pricies;

  Result ret;

  // ローソク足データ取得
#ifdef X2_LOGIC
  ret = get_kline(buffer, symbol, "4h", 0, server_unix_time, s_range * 2 * th);
#else
  ret = get_kline(buffer, symbol, "4h", 0, server_unix_time, s_range * th);
#endif
  if (ret != Result::Success) {
      PLOG_ERROR << "Failed to get ema.";
      return -1;
  }

  c_pricies = get_c_pricies_from_klines(buffer);
  s_ema = get_ema(c_pricies, s_range);
  c_pricies.clear();
  buffer.clear();

#ifdef X2_LOGIC
  ret = get_kline(buffer, symbol, "4h", 0, server_unix_time, l_range * 2 * th);
#else
  ret = get_kline(buffer, symbol, "4h", 0, server_unix_time, l_range * th);
#endif
  if (ret != Result::Success) {
      PLOG_ERROR << "Failed to get ema.";
      return -1;
  }

  c_pricies = get_c_pricies_from_klines(buffer);
  l_ema = get_ema(c_pricies, l_range);
  c_pricies.clear();
  buffer.clear();

  return s_ema - l_ema;
}

std::vector<float>
ApiStore::get_c_pricies_from_klines(Json::Value& result)
{
  std::vector<float> list;

  for (auto i = 0; i < result.size(); ++i) {
    list.push_back(std::stof(result[i][4].asString()));
  }

  return list;
}

float
ApiStore::get_ema(std::vector<float>& c_pricies, int range)
{
  // 平滑化定数
  const float ALPHA = 2.0f / (range + 1);

  // 1 日目の SMA を算出 (= 1 日目の EMA)
  float y_ema = 0.0f;
#ifdef X2_LOGIC
  for (auto i = 0; i < c_pricies.size() / 2; ++i) {
#else
  for (auto i = 0; i < c_pricies.size(); ++i) {
#endif
    y_ema+= c_pricies[i];
  }
  y_ema /= range;

  float t_ema = 0.0f;
#ifdef X2_LOGIC
  for (auto i = c_pricies.size() / 2; i < c_pricies.size(); ++i) {
#else
  for (auto i = 1; i < c_pricies.size(); ++i) {
#endif
    t_ema = y_ema + ALPHA * (c_pricies[i] - y_ema);
    y_ema = t_ema;
  }

  return t_ema;
}

Result
ApiStore::get_kline(
    Json::Value& result,
    const char* symbol,
    const char* interval_day,
    long open_unix_time,
    long close_unix_time,
    int limit
)
{
  int retry = API_CALL_LEFT;

  while (1) {
    result.clear();
    binanceError_t ret = m_market.getKlines(result, symbol, interval_day, open_unix_time, close_unix_time, limit);
    if (ret == binanceSuccess) {
      break;
    } else {
      PLOG_WARNING.printf("Failed to request. Try Again (Last: %d)", retry); 
      if (--retry <= 0) {
        PLOG_ERROR << "Failed to get KLine.";
        return Result::Failed;
      }
    }
  }

  return Result::Success;
}

float
ApiStore::get_balance(const char* symbol)
{
  Json::Value result;
  long recv_window = 10000;

  int retry = API_CALL_LEFT;

  while (1) {
    result.clear();
    binanceError_t ret = m_account.getWalletData(result);
    if (ret == binanceSuccess) {
      break;
    } else {
      if (--retry <= 0) {
        PLOG_ERROR << "Falied to get balance.";
        return -1;
      }
    }
  }

  float free = -1.0f;
  for (const Json::Value& coin_info : result) {
    if (coin_info["coin"].asString() == symbol) {
      free = std::stof(coin_info["free"].asString());
    }
  }

  return free;
}

Result
ApiStore::purchase(const char* symbol, float balance)
{
  Json::Value buffer;
  binanceError_t ret;

  int retry = API_CALL_LEFT;
  double price;
  double qty;

  price = get_price(symbol);
  if (price < 0) {
    return Result::Failed;
  }

  // 0.9 は確実に買う為の保険
  qty = (balance / price) * 0.90;
  qty = MathUtils::round_n(qty, 5);

  PLOG_INFO.printf("qty: %f", qty);

  retry = API_CALL_LEFT;

  while (retry > 0) {
    ret = m_account.sendOrder(buffer, symbol, "BUY", "MARKET", "", qty, 0, "0", 0, 0, RECV_WINDOW);
    if (ret == binanceSuccess) {
      break;
    } else {
      if (--retry <= 0) goto _FAIL;
    }
  }

  return Result::Success;

_FAIL:
  PLOG_ERROR << "Failed to purchase.";
  return Result::Failed;
}

Result
ApiStore::sell(const char* symbol, float balance)
{
  Json::Value buffer;
  binanceError_t ret;

  int retry = API_CALL_LEFT;
  double qty;

  // 0.95 は確実に売る為の保険
  qty = MathUtils::round_n(balance * 0.95, 5);
  PLOG_INFO.printf("qty: %f", qty);

  retry = API_CALL_LEFT;

  while (retry > 0) {
    ret = m_account.sendOrder(buffer, symbol, "SELL", "MARKET", "", qty, 0, "0", 0, 0, RECV_WINDOW);
    if (ret == binanceSuccess) {
      break;
    } else {
      if (--retry < 0) goto _FAIL;
    }
  }

  return Result::Success;

_FAIL:
  PLOG_ERROR << "Failed to sell.";
  return Result::Failed;
}

double
ApiStore::get_price(const char* symbol)
{
  int retry = API_CALL_LEFT;

  double price = 0;
  while (retry > 0) {
    binanceError_t ret = m_market.getPrice(symbol, price);
    if (ret == binanceSuccess) {
      break;
    } else {
      if (--retry < 0) return -1;
    }
  }

  return price;
}
