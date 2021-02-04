#pragma once

#include "config_accessor.h"
#include "transaction_manager.h"

class TransactionManager;

class StateManager {
  public:
    enum class State {
      PURCHASED = -3,
      PURCHASE  = -2,
      READY_P   = -1,
      IDLE      = 0,
      READY_S   = 1,
      SELL      = 2,
      SOLD      = 3
    };

    StateManager(ConfigAccessor* config, TransactionManager* manager);
    ~StateManager();

    Result exec(float macd_value, float signal_value);
  private:
    TransactionManager* m_manager;
    StateManager::State m_cur_state;

    int m_wait_limit;
    int m_waited_count;
};
