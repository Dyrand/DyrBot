#ifndef PRIVMSG_PARSER
#define PRIVMSG_PARSER

#include "../structs/message_struct.hpp"
#include "../structs/privmsg_struct.hpp"

namespace dyr
{
  Privmsg_Struct parsePrivmsg(const Message_Struct& message, const std::string& unqiue_delim);
}

#endif /*PRIVMSG_PARSER*/
