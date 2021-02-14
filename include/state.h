#pragma once

#include "config_accessor.h"
#include "transaction_manager.h"
#include "signal.h"

class TransactionManager;

class StateManager {
 public:
  enum class State {
    PURCHASE  = -2,
    READY_P   = -1,
    IDLE      = 0,
    READY_S   = 1,
    SELL      = 2,
  };

  StateManager(ConfigAccessor* config, TransactionManager* manager);
  ~StateManager();

  Result exec(TransactionSignal macd_sig, TransactionSignal cci_sig);
 private:
  TransactionManager* m_manager;
  StateManager::State m_cur_state;

  int m_wait_limit;
  int m_waited_count;
};
