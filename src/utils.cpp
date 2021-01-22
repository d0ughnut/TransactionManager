#include <iostream>
#include <plog/Log.h>

#include "utils.h"

Utils::Utils()
{
  // private
}

Utils::~Utils()
{
  // private
}

bool
Utils::is_exist(const std::string& path)
{
  struct stat sb;
  return (stat(path.c_str(), &sb) == 0);
}

Result
Utils::make_directory(const std::string& path)
{
  if (mkdir(path.c_str(), 0777) != 0) {
    PLOG_ERROR << "failed to create dir";
    return Result::Failed;
  }

  return Result::Success;
}
