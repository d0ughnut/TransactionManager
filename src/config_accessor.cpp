#include <plog/Log.h>
#include <iostream>

#include "config_accessor.h"
#include "result.h"

const char* ConfigAccessor::API_KEY_FILE_PATH = "/etc/binance/config.ini";

ConfigAccessor::ConfigAccessor() {
  m_file = g_key_file_new();
}

ConfigAccessor::~ConfigAccessor() {
  if (m_file) {
    g_key_file_free(m_file);
  }
}

Result
ConfigAccessor::load_file() {
  if (!m_file) {
    PLOG_ERROR << "g_key_file is empty.";
    return Result::Failed;
  }

  gboolean ret = g_key_file_load_from_file(
      m_file,
      ConfigAccessor::API_KEY_FILE_PATH,
      G_KEY_FILE_NONE,
      NULL
  );

  if (ret != TRUE) {
    PLOG_ERROR << "failed to load <config.ini>.";
    return Result::Failed;
  }

  return Result::Success;
}

int
ConfigAccessor::get_transaction_wait_count() {
  return get_integer_value("transaction", "wait_count");
}

std::string
ConfigAccessor::get_client_addr() {
  return get_string_value("client", "addr");
}

int
ConfigAccessor::get_client_port() {
  return get_integer_value("client", "port");
}

std::string
ConfigAccessor::get_api_key() {
  return get_string_value("api", "key");
}

std::string
ConfigAccessor::get_api_secret() {
  return get_string_value("api", "secret");
}

int
ConfigAccessor::get_api_request_interval_sec() {
  return get_integer_value("api", "wait_sec");
}

int
ConfigAccessor::get_macd_short_param() {
  return get_integer_value("macd", "short");
}

int
ConfigAccessor::get_macd_long_param() {
  return get_integer_value("macd", "long");
}

int
ConfigAccessor::get_signal_param() {
  return get_integer_value("macd", "signal");
}

std::string
ConfigAccessor::get_purchase_from_symbol() {
  return get_string_value("transaction", "purchase_from");
}

std::string
ConfigAccessor::get_sell_to_symbol() {
  return get_string_value("transaction", "sell_to");
}

int
ConfigAccessor::get_cci_length() {
  return get_integer_value("cci", "length");
}

std::string
ConfigAccessor::get_string_value(const char* group, const char* key) {
  gchar* value = g_key_file_get_string(
      m_file,
      group,
      key,
      NULL
  );

  if (!value) {
    PLOG_ERROR << "Failed to get string value from config.";
    return std::string("");
  }

  return std::string(value);
}

int
ConfigAccessor::get_integer_value(const char* group, const char* key) {
  gint value = g_key_file_get_integer(
      m_file,
      group,
      key,
      NULL
  );

  if (!value) {
    PLOG_ERROR << "Failed to get integer value from config.";
    return -1;
  }

  return value;
}
