#pragma once

#include <string>

#include "result.h"

class Utils {
  public:
    static Result make_directory(const std::string& path);
    static bool is_exist(const std::string& path);

  private:
    Utils();
    ~Utils();
};
