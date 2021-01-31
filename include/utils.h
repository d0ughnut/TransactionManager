#pragma once

#include <string>

#include "result.h"

namespace FileUtils {
  Result make_directory(const std::string& path);
  bool is_exist(const std::string& path);
}

namespace MathUtils {
  double round_n(double number, int n);
}
