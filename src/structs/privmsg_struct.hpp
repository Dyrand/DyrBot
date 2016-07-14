#ifndef PRIVMSG_STRUCT_HPP
#define PRIVMSG_STRUCT_HPP

#include <map>
#include <vector>
#include <string>

namespace dyr
{
  struct Privmsg_Struct
  {
    std::string nickname;
    std::string username;
    std::string target;
    std::string ident;
    std::vector<std::string> command;
    std::vector<std::string> after_command;
    std::vector<std::map<std::string,std::vector<std::string> > > arguments;
  };
}

#endif /*PRIVMSG_STRUCT_HPP*/
