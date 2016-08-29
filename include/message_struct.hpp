#ifndef MESSAGE_STRUCT_HPP
#define MESSAGE_STRUCT_HPP

#include <string>

namespace dyr
{
  /*IRC Messages are divided into three parts: Prefix, Command, and Parameters*/
  struct irc_message_struct
  {
    std::string prefix;
    std::string command;
    std::string parameters;
  };
}

#endif /*MESSAGE_STRUCT_HPP*/
