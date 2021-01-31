#include <iostream>
#include <cmath>
#include <plog/Log.h>

#include "utils.h"

namespace FileUtils {

bool is_exist(const std::string& path)
{
  struct stat sb;
  return (stat(path.c_str(), &sb) == 0);
}

Result make_directory(const std::string& path)
{
  if (mkdir(path.c_str(), 0777) != 0) {
    PLOG_ERROR << "failed to create dir";
    return Result::Failed;
  }

  return Result::Success;
}

}

namespace MathUtils {

double round_n(double number, int n)
{
  number = number * pow(10, n - 1);
  number = round(number);
  number /= pow(10, n - 1);
  return number;
}

}
