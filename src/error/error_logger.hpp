#ifndef ERROR_LOGGER_HPP
#define ERROR_LOGGER_HPP

#include <fstream>

class logError
{
  public:
    static void toFile(const std::string& text);

  private:
    logError(){} //Objectless class
};

#endif /*ERROR_LOGGER_HPP*/
