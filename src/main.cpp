#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include <memory>
#include <string>

#include "result.h"
#include "transaction_manager.h"
#include "alias.h"
#include "constant.h"

int main(int argc, char* argv[]) {
  Result result;

  static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;
  plog::init(plog::info, &console_appender);

  std::unique_ptr<TransactionManager> manager;

  Property::State init_state = Property::State::IDLE;
  if (argc == 2) {
    std::string arg_str = std::string(argv[1]);
    if (arg_str == "PURCHASE") {
      init_state = Property::State::PURCHASE;
    } else if (arg_str == "SELL") {
      init_state = Property::State::SELL;
    } else if (arg_str == "READY_P") {
      init_state = Property::State::READY_P;
    } else if (arg_str == "READY_S") {
      init_state = Property::State::READY_S;
    } else {
      PLOG_FATAL << "Invalid arg.";
      goto _FAIL;
    }
  } else if (argc > 2) {
    PLOG_FATAL << "Invalid args.";
    goto _FAIL;
  }

  PLOG_INFO << "Booted.";

  manager = std::make_unique<TransactionManager>();
  if (!manager) {
    PLOG_FATAL << "Failed to allocate <TransactionManager>.";
    goto _FAIL;
  }

  result = manager->initialize();
  if (result != Result::Success) goto _FAIL;

  // Blocking
  manager->entry(init_state);
  manager->exec();

  PLOG_INFO <<  "Terminated.";

  return 0;

_FAIL:
  PLOG_FATAL << "Abend.";
  return 1;
}
