#ifndef MODULE_CUSTODIAN_COMMANDS_HPP
#define MODULE_CUSTODIAN_COMMANDS_HPP

#include <string>

#include "../module.hpp"
#include "../../structs/privmsg_struct.hpp"

namespace dyr
{
  namespace module
  {
    module::Module custodian;
    bool see_r(module::Module* parent, Privmsg_Struct& msg);
    std::string see(module::Module* parent,Privmsg_Struct& msg, int index);
  }
}

#endif /*MODULE_CUSTODIAN_COMMANDS_HPP*/
