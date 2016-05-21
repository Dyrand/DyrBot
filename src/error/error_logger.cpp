#include <fstream>

#include "error_logger.hpp"

void logError::toFile(const std::string& text)
{
  static std::ofstream errout("error.log");
  errout << text << '\n';
}
