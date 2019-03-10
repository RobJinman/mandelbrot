#include <iomanip>
#include <wx/stdpaths.h>
#include "utils.hpp"
#include "config.hpp"

using std::string;

string versionString() {
  std::stringstream ss;
  ss << "Mandelbrot " << Mandelbrot_VERSION_MAJOR << "."
     << Mandelbrot_VERSION_MINOR;
  return ss.str();
}

string appDataPath(const string& relPath) {
  auto standardPaths = wxStandardPaths::Get();

  if (relPath.empty()) {
    return standardPaths.GetDataDir().ToStdString();
  }

  return joinPaths(standardPaths.GetDataDir(), relPath);
}

string userDataPath(const string& relPath) {
  auto standardPaths = wxStandardPaths::Get();

  if (relPath.empty()) {
    return standardPaths.GetUserDataDir().ToStdString();
  }

  return joinPaths(standardPaths.GetUserDataDir(), relPath);
}

string formatDouble(double d) {
  std::stringstream ss;
  ss << std::scientific << d;
  return ss.str();
}
