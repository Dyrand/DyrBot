#ifndef MESSAGE_STRUCT_HPP
#define MESSAGE_STRUCT_HPP

#include <string>

namespace dyr
{
  /*IRC Messages are divided into three parts: Prefix, Command, and Parameters*/
  struct Message_Struct
  {
    std::string prefix;
    std::string command;
    std::string parameters;
  };
}

#endif /*MESSAGE_STRUCT_HPP*/
