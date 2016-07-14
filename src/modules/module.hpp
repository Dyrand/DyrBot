#ifndef MODULES_MODULE_HPP
#define MODULES_MODULE_HPP

#include <string>
#include <vector>
#include <map>

#include <boost/weak_ptr.hpp>

#include "../structs/privmsg_struct.hpp"

namespace dyr
{
  class DyrBot;

  namespace module
  {
    struct restricted_command
    {
      std::function<bool(Privmsg_Struct&)> restriction;
      std::function<std::string(Privmsg_Struct&,int)> command;
    };

    class Module
    {
      public:
        void init(boost::shared_ptr<DyrBot> botptr);
        std::string version;
        std::map<std::string,restricted_command> commands;
        boost::weak_ptr<DyrBot> dyrbot;
    };

    class ModuleManager
    {
      public:
        static ModuleManager* get()
        {
          static ModuleManager singleton;
          return &singleton;
        }
        static void InitializeModules();
        static std::vector<Module> module_vec;
      private:
        ModuleManager(){};
    };

    bool no_restriction()
    {return false;}
  }
}

#endif /*MODULE_HPP*/
