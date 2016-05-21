#include <boost/algorithm/string.hpp>

#include "../structs/message_struct.hpp"

namespace dyr
{
  Message_Struct parseMessage(std::string message)
  {
    namespace algo = boost::algorithm;

    if(!message.empty())
    {
      Message_Struct message_struct;

      auto space_pos = message.find(' ',0);

      if(message.front() == ':')
      {
        message_struct.prefix = message.substr(0, space_pos);
        message.erase(0,space_pos+1);
      }

      space_pos = message.find(' ',0);
      message_struct.command = message.substr(0, space_pos);
      message.erase(0,space_pos+1);

      message_struct.raw_parameters = message;

      return message_struct;
    }

    return Message_Struct();
  }
}
