#include <functional>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "module.hpp"

//Module headers
#include "custodian/commands.hpp"

namespace dyr
{
  namespace module
  {
    using namespace std::placeholders;

    void Module::init(boost::shared_ptr<DyrBot> botptr)
    { dyrbot = botptr; }

    void ModuleManager::InitializeModules()
    {
      restricted_command see_rc
      {
        std::bind(see_r,&custodian,_1),
        std::bind(see,&custodian,_1,_2)      
      };

      custodian.version = "1.0";
      custodian.commands.at("see") = see_rc;
      module_vec.emplace_back(std::move(custodian));
    }
  }
}
