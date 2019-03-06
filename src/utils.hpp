#pragma once

#include <iostream>
#include <string>

#define DBG_PRINT_VAR(x) \
  { \
    std::cout << #x << "=" << x << std::endl; \
  }

std::string versionString();
