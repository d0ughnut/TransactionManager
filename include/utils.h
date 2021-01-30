#pragma once

#include <string>

#include "result.h"

class Utils {
  public:
    static Result make_directory(const std::string& path);
    static bool is_exist(const std::string& path);
    static double round_n(double number, int n);

  private:
    Utils();
    ~Utils();
};
