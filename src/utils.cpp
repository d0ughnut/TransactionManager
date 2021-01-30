#include <iostream>
#include <cmath>
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

double
Utils::round_n(double number, int n)
{
  number = number * pow(10,n-1); //四捨五入したい値を10の(n-1)乗倍する。
  number = round(number); //小数点以下を四捨五入する。
  number /= pow(10, n-1); //10の(n-1)乗で割る。
  return number;
}
