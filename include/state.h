#pragma once

#include "config_accessor.h"
#include "transaction_manager.h"
#include "signal.h"
#include "constant.h"

class TransactionManager;

class StateManager {
 public:
  StateManager(ConfigAccessor* config, TransactionManager* manager);
  ~StateManager();

  void entry(Property::State init_state);
  Result exec(TransactionSignal macd_sig, TransactionSignal cci_sig);

 private:
  TransactionManager* m_manager;
  Property::State m_cur_state;

  int m_wait_limit;
  int m_waited_count;
};
