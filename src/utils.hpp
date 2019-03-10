#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#define DBG_PRINT_VAR(x) \
  { \
    std::cout << #x << "=" << x << std::endl; \
  }

#ifdef WIN32
const char SEPARATOR = '\\';
#else
const char SEPARATOR = '/';
#endif

std::string versionString();
std::string appDataPath(const std::string& relPath = "");
std::string userDataPath(const std::string& relPath = "");

template <typename T_FIRST, typename ...T_REST,
          typename = std::common_type_t<T_FIRST, T_REST...>>
std::string joinPaths(T_FIRST first, T_REST... rest) {
  std::vector<T_FIRST> parts{rest...};
  std::stringstream ss;

  ss << first;
  for (const T_FIRST& part : parts) {
    ss << SEPARATOR << part;
  }
  return ss.str();
}
