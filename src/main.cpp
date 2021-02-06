#include <memory>
#include <plog/Log.h>
#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include "result.h"
#include "transaction_manager.h"

int main()
{
  Result result;

  static plog::ColorConsoleAppender<plog::TxtFormatter> console_appender;
  plog::init(plog::info, &console_appender);

  PLOG_INFO << "Booted.";

  std::unique_ptr<TransactionManager> manager = std::make_unique<TransactionManager>();
  if (!manager) {
    PLOG_FATAL << "Failed to allocate <TransactionManager>.";
    goto _FAIL;
  }

  result = manager->initialize();
  if (result != Result::Success) goto _FAIL;

  manager->exec();

  PLOG_INFO <<  "Terminated.";

  return 0;

_FAIL:
  PLOG_FATAL << "Abend.";
  return 1;
}
