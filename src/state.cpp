#include <plog/Log.h>
#include <iostream>

#include "state.h"
#include "signal.h"

StateManager::StateManager(
    ConfigAccessor* config,
    TransactionManager* manager
) : m_manager(manager),
    m_cur_state(StateManager::State::IDLE),
    m_wait_limit(config->get_transaction_wait_count()),
    m_waited_count(0) {
  if (m_wait_limit < 0) {
    PLOG_WARNING << "Invalid param (wait_count), so use default val (5).";
    m_wait_limit = 5;
  }
  PLOG_INFO.printf("Transaction wait: %d", m_wait_limit);
}

StateManager::~StateManager() {
}

Result
StateManager::exec(TransactionSignal macd_sig, TransactionSignal cci_sig) {
  bool should_purchase = (macd_sig == TransactionSignal::PURCHASE);

  StateManager::State next_state = m_cur_state;

  // Do
  switch (m_cur_state) {
    case StateManager::State::IDLE:
      // State: IDLE
      //   cci のみ待受 (macd は無視)
      //   cci に売買シグナルが出ている場合は準備状態に遷移
      switch (cci_sig) {
        case TransactionSignal::PURCHASE:
          next_state = StateManager::State::READY_P;
          break;
        case TransactionSignal::SELL:
          next_state = StateManager::State::READY_S;
          break;
      }
      break;
    case StateManager::State::PURCHASE:
    case StateManager::State::SELL:
      // State: PURCASE, SELL
      //   注文後に一度だけ通る
      //   動きは IDLE と同じでイベントが無ければ IDLE に遷移
      switch (cci_sig) {
        case TransactionSignal::PURCHASE:
          next_state = StateManager::State::READY_P;
          break;
        case TransactionSignal::SELL:
          next_state = StateManager::State::READY_S;
          break;
        default:
          next_state = StateManager::State::IDLE;
          break;
      }
      break;
    case StateManager::State::READY_P:
      // State: READY_P
      //   買い注文の待機状態
      //   macd が買いシグナルを出すまで待機する
      //   待機中に cci のシグナルが反転したら READY_S へ遷移
      if (cci_sig == TransactionSignal::SELL) {
        // cci が売りシグナルを出した場合は即時売り準備状態に遷移
        next_state = StateManager::State::READY_S;
        break;
      }

      switch (macd_sig) {
        case TransactionSignal::PURCHASE:
          if (++m_waited_count >= m_wait_limit) {
            next_state = StateManager::State::PURCHASE;
          }
          break;
        case TransactionSignal::SELL:
          // カウンタをリセット
          m_waited_count = 0;
          break;
      }

      break;
    case StateManager::State::READY_S:
      // State: READY_S
      //   売り注文の待機状態
      //   macd が売りシグナルを出すまで待機する
      //   待機中に cci のシグナルが反転したら READY_P へ遷移
      if (cci_sig == TransactionSignal::PURCHASE) {
        // cci が売りシグナルを出した場合は即時売り準備状態に遷移
        next_state = StateManager::State::READY_P;
        break;
      }

      switch (macd_sig) {
        case TransactionSignal::SELL:
          if (++m_waited_count >= m_wait_limit) {
            next_state = StateManager::State::PURCHASE;
          }
          break;
        case TransactionSignal::PURCHASE:
          // カウンタをリセット
          m_waited_count = 0;
          break;
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
