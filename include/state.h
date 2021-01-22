#pragma once

#include "config_accessor.h"

class StateManager {
  public:
    enum class State {
      BOUGHT    = -3,
      BUY       = -2,
      READY_B   = -1,
      IDLE      = 0,
      READY_S   = 1,
      SELL      = 2,
      SOLD      = 3
    };

    StateManager(ConfigAccessor* config);
    ~StateManager();

    StateManager::State get_next_state(bool should_buy);
  private:
    StateManager::State m_cur_state;

    int m_wait_limit;
    int m_waited_count;
};
