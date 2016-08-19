#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <utility>
#include <fstream>
#include <iomanip>
#include <string>
#include <ctime>

class log
{
  public:
    //Replaces all %i with actual integers provided in the variable arguments
    template<typename T, typename... Targs>
    static void toFile(const char* format, T value, Targs... Fargs)
    {
      auto t = std::time(nullptr);
      auto tm = *std::localtime(&t);
      fileout << std::put_time(&tm, "{%d-%m-%Y %H:%M:%S} ");

      toFileHelper(format, value, Fargs...);

      fileout << "\n";
    }

    static void toFile(const std::string& text);
    static void toConsole(const std::string& text);

    static void openFile(const std::string& output_file);
    static void closeFile();

    /* Set to true if the creation of the log file is
       not meant to be logged into the log file */
    static void setSilentCreation(bool silent);

  private:
    log() = delete; //Objectless class

    template<typename T, typename... Targs>
    static void toFileHelper(const char* format, T value, Targs... Fargs)
    {
      while(*format != '\0')
      {
        if( *format == '%' )
        {
          fileout << value;
          toFileHelper(format+1, Fargs...);
          return;
        }
        fileout << *format;
        ++format;
      }
    }

    static void toFileHelper(const char* format)
    {
      while(*format != '\0')
      {
        fileout << *format;
        ++format;
      }
    }

    static bool silentCreation;

    static std::string filename;
    static std::ofstream fileout;
};

#endif /*LOGGER_HPP*/
