#include <plog/Log.h>
#include <iostream>

#include "state.h"
#include "signal.h"

StateManager::StateManager(
  ConfigAccessor* config,
  TransactionManager* manager
) :
m_manager(manager),
m_cur_state(StateManager::State::IDLE),
m_wait_limit(config->get_transaction_wait_count()),
m_waited_count(0)
{
  if (m_wait_limit < 0) {
    PLOG_WARNING << "Invalid param (wait_count), so use default val (5).";
    m_wait_limit = 5;
  }
  PLOG_INFO.printf("Transaction wait: %d", m_wait_limit);
}

StateManager::~StateManager()
{
}

Result
StateManager::exec(Signal macd_sig, Signal cci_sig)
{
  bool should_purchase = (macd_sig == Signal::PURCHASE);

  StateManager::State next_state = m_cur_state;

  // Do
  switch (m_cur_state) {
    case StateManager::State::IDLE:
      if (should_purchase) {
        next_state = StateManager::State::READY_P;
      } else {
        next_state = StateManager::State::READY_S;
      }
      break;
    case StateManager::State::PURCHASE:
      if (should_purchase) {
        next_state = StateManager::State::PURCHASED;
      } else {
        next_state = StateManager::State::READY_P;
      }
      break;
    case StateManager::State::SELL:
      if (should_purchase) {
        next_state = StateManager::State::READY_S;
      } else {
        next_state = StateManager::State::SOLD;
      }
      break;
    case StateManager::State::PURCHASED:
      if (should_purchase) {
        next_state = StateManager::State::PURCHASED;
      } else {
        next_state = StateManager::State::READY_S;
      }
      break;
    case StateManager::State::SOLD:
      if (should_purchase) {
        next_state = StateManager::State::READY_P;
      } else {
        next_state = StateManager::State::SOLD;
      }
      break;
    case StateManager::State::READY_P:
      if (should_purchase) {
        if (++m_waited_count >= m_wait_limit) {
          next_state = StateManager::State::PURCHASE;
        }
      } else {
        next_state = StateManager::State::READY_S;
      }
      break;
    case StateManager::State::READY_S:
      if (should_purchase) {
        next_state = StateManager::State::READY_P;
      } else {
        if (++m_waited_count >= m_wait_limit) {
          next_state = StateManager::State::SELL;
        }
      }
      break;
  }

  // Exit
  Result ret = Result::Success;
  if (m_cur_state != next_state) {
    // カウンタをリセット
    m_waited_count = 0;
    PLOG_INFO.printf("NextState: %d", static_cast<int>(next_state));

    // 取引をリクエスト
    switch (next_state) {
      case StateManager::State::PURCHASE:
        ret = m_manager->purchase();
        break;
      case StateManager::State::SELL:
        ret = m_manager->sell();
        break;
    }

    m_cur_state = next_state;
  }

  return ret;
}
