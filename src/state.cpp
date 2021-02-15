#include <plog/Log.h>
#include <iostream>

#include "state.h"
#include "signal.h"
#include "constant.h"

StateManager::StateManager(
    ConfigAccessor* config,
    TransactionManager* manager
) : m_manager(manager),
    m_cur_state(Property::State::IDLE),
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

void
StateManager::entry(Property::State init_state) {
  m_cur_state = init_state;
}

Result
StateManager::exec(TransactionSignal macd_sig, TransactionSignal cci_sig) {
  Result ret = Result::Success;
  Property::State next_state = m_cur_state;

  if (cci_sig == macd_sig) {
    // 待機不要な場合 (cci がシグナルを出した時点で既に macd が交差している場合)
    // 即時注文をリクエストする
    switch (cci_sig) {
      case TransactionSignal::PURCHASE:
        m_cur_state = Property::State::PURCHASE;
        break;
      case TransactionSignal::SELL:
        m_cur_state = Property::State::SELL;
        break;
    }
  } else {
    // cci から強いシグナルが出ている場合は即時注文する
    switch (cci_sig) {
      case TransactionSignal::F_PURCHASE:
        m_cur_state = Property::State::PURCHASE;
        break;
      case TransactionSignal::F_SELL:
        m_cur_state = Property::State::SELL;
        break;
    }
  }

  // Entry
  // 取引をリクエスト
  switch (m_cur_state) {
    case Property::State::PURCHASE:
      ret = m_manager->purchase();
      break;
    case Property::State::SELL:
      ret = m_manager->sell();
      break;
  }

  // Do
  switch (m_cur_state) {
    case Property::State::IDLE:
      // State: IDLE
      //   cci のみ待受 (macd は無視)
      //   cci に売買シグナルが出ている場合は準備状態に遷移
      switch (cci_sig) {
        case TransactionSignal::PURCHASE:
          next_state = Property::State::READY_P;
          break;
        case TransactionSignal::SELL:
          next_state = Property::State::READY_S;
          break;
      }
      break;
    case Property::State::PURCHASE:
    case Property::State::SELL:
      // State: PURCHASE, SELL
      //   注文後に一度だけ通る
      //   動きは IDLE と同じでイベントが無ければ IDLE に遷移
      switch (cci_sig) {
        case TransactionSignal::PURCHASE:
          next_state = Property::State::READY_P;
          break;
        case TransactionSignal::SELL:
          next_state = Property::State::READY_S;
          break;
        default:
          next_state = Property::State::IDLE;
          break;
      }
      break;
    case Property::State::READY_P:
      // State: READY_P
      //   買い注文の待機状態
      //   macd が買いシグナルを出すまで待機する
      //   待機中に cci のシグナルが反転したら READY_S へ遷移
      if (cci_sig == TransactionSignal::SELL) {
        // cci が売りシグナルを出した場合は即時売り準備状態に遷移
        next_state = Property::State::READY_S;
        break;
      }

      switch (macd_sig) {
        case TransactionSignal::PURCHASE:
          if (++m_waited_count >= m_wait_limit) {
            next_state = Property::State::PURCHASE;
          }
          break;
        case TransactionSignal::SELL:
          // カウンタをリセット
          m_waited_count = 0;
          break;
      }

      break;
    case Property::State::READY_S:
      // State: READY_S
      //   売り注文の待機状態
      //   macd が売りシグナルを出すまで待機する
      //   待機中に cci のシグナルが反転したら READY_P へ遷移
      if (cci_sig == TransactionSignal::PURCHASE) {
        // cci が買いシグナルを出した場合は即時買い準備状態に遷移
        next_state = Property::State::READY_P;
        break;
      }

      switch (macd_sig) {
        case TransactionSignal::SELL:
          if (++m_waited_count >= m_wait_limit) {
            next_state = Property::State::SELL;
          }
          break;
        case TransactionSignal::PURCHASE:
          // カウンタをリセット
          m_waited_count = 0;
          break;
      }

      break;
  }

_SKIP:

  // Exit
  if (m_cur_state != next_state) {
    // カウンタをリセット
    m_waited_count = 0;
    PLOG_INFO.printf("NextState: %d", static_cast<int>(next_state));

    m_cur_state = next_state;
  }

  return ret;
}
