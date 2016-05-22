#ifndef PARSERS_HPP
#define PARSERS_HPP

#include "../structs/message_struct.hpp"
#include "../structs/privmsg_struct.hpp"

namespace dyr
{
  Message_Struct parseMessage(std::string message);
  Privmsg_Struct parsePrivmsg(const Message_Struct& message, const std::string& unqiue_delim);
}

#endif /*PARSERS_HPP*/
