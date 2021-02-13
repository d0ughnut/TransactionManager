#pragma once

#include <glib.h>
#include <string>

#include "result.h"


class ConfigAccessor {
  public:
    ConfigAccessor();
    ~ConfigAccessor();

    Result load_file();

    int get_transaction_wait_count();
    std::string get_client_addr();
    int get_client_port();
    std::string get_api_key();
    std::string get_api_secret();
    int get_api_request_interval_sec();
    std::string get_purchase_from_symbol();
    std::string get_sell_to_symbol();
    int get_macd_short_param();
    int get_macd_long_param();
    int get_signal_param();
    int get_cci_length();

  private:
    std::string get_string_value(const char* group, const char* key);
    int get_integer_value(const char* group, const char* key);

    static const char* API_KEY_FILE_PATH;
    GKeyFile* m_file;
};
