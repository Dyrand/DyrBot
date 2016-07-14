#include <fstream>
#include <iomanip>
#include <ctime>

#include "error_logger.hpp"

void logError::toFile(const std::string& text)
{
  static std::ofstream errout("error.log");
  auto t = std::time(nullptr);
  auto tm = *std::localtime(&t);
  errout << std::put_time(&tm, "{%d-%m-%Y %H:%M:%S} ") << text << '\n';
}
