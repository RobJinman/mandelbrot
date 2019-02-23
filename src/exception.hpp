#pragma once

#include <stdexcept>
#include <sstream>

#define EXCEPTION(msg) \
  { \
    std::stringstream ss; \
    ss << msg << " (FILE: " << __FILE__ << ", LINE: " << __LINE__ << ")" \
       << std::endl; \
    throw std::runtime_error(ss.str()); \
  }
