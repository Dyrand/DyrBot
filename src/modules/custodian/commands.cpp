#include <string>

#include <boost/shared_ptr.hpp>

#include "../module.hpp"
#include "../../structs/privmsg_struct.hpp"
#include "../../DyrBot.hpp"

namespace dyr
{
  namespace module
  {
    bool see_r(module::Module* parent, Privmsg_Struct& msg)
    {
      if(boost::shared_ptr<DyrBot> shr_ptr = parent->dyrbot.lock())
      {
        if(shr_ptr->var["custodian"] == msg.nickname)
        { return false; }
        return true;
      }
      else
      {return true;}
    }
    std::string see(module::Module* parent, Privmsg_Struct& privmsg, int index)
    {
      if(msg.arguments.at(index)[""].size() >= 1)
      {
        std::string temp;
        for(auto& str: msg.arguments.at(index)[""])
        { temp += str + "=\"" + var[str] + "\" "; }
        dyrbot->privmsg(msg.target,temp);
      }
      return "";
    }
  }
}
