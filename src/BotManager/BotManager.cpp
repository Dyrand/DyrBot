#include <utility>

#include "BotManager.hpp"
#include "UnqiueIdentifier.hpp"

namespace dyr
{
  bool BotManager::createBot()
  {
    bool success = false;

    int id = uuid::generate();

    if( id != -1)
    {
      auto emplace_status = id_bot_map.try_emplace(id); //Replace with actual args
                                                        //for DyrBot
      success = emplace_status.second;
    }

    return success;
  }

  bool BotManager::deleteBot(int id)
  {
    bool success = false;
  }
}
