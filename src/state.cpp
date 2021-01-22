#include <iostream>
#include <plog/Log.h>

#include "state.h"

StateManager::StateManager(ConfigAccessor* config) :
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

StateManager::State
StateManager::get_next_state(bool should_buy) {

  StateManager::State next_state = m_cur_state;

  switch (m_cur_state) {
    case StateManager::State::IDLE:
      if (should_buy) {
        next_state = StateManager::State::READY_B;
      } else {
        next_state = StateManager::State::READY_S;
      }
      break;
    case StateManager::State::BUY:
      if (should_buy) {
        next_state = StateManager::State::BOUGHT;
      } else {
        next_state = StateManager::State::READY_B;
      }
      break;
    case StateManager::State::SELL:
      if (should_buy) {
        next_state = StateManager::State::READY_S;
      } else {
        next_state = StateManager::State::SOLD;
      }
      break;
    case StateManager::State::BOUGHT:
      if (should_buy) {
        next_state = StateManager::State::BOUGHT;
      } else {
        next_state = StateManager::State::READY_S;
      }
      break;
    case StateManager::State::SOLD:
      if (should_buy) {
        next_state = StateManager::State::READY_B;
      } else {
        next_state = StateManager::State::SOLD;
      }
      break;
    case StateManager::State::READY_B:
      if (should_buy) {
        if (++m_waited_count >= m_wait_limit) {
          next_state = StateManager::State::BUY;
        }
      } else {
        next_state = StateManager::State::READY_S;
      }
      break;
    case StateManager::State::READY_S:
      if (should_buy) {
        next_state = StateManager::State::READY_B;
      } else {
        if (++m_waited_count >= m_wait_limit) {
          next_state = StateManager::State::SELL;
        }
      }
      break;
  }

  if (m_cur_state != next_state) {
    // reset
    m_waited_count = 0;
    m_cur_state = next_state;
    PLOG_INFO.printf("NextState: %d", static_cast<int>(m_cur_state));
  }

  return m_cur_state;
}
