#include <utility>
#include <tuple>
#include <map>

#include "UniqueIdentifier.hpp"
#include "Logger.hpp"
#include "BotManager.hpp"
#include "DyrBot.hpp"

namespace dyr
{
  int BotManager::getBotCount()
  {
    return id_bot_map.size();
  }

  /*Returns the uuid of the bot created
    -1 otherwise */
  int BotManager::createBot()
  {
    int id = uuid::generate();

    if( id != -1 )
    {
      std::pair<int,DyrBot> id_bot_pair(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple()
      );

      auto emplace_status = id_bot_map.insert(std::move(id_bot_pair));

      #ifdef DEBUG
        log::toFile("DyrBot created with UUID{%}", id);
      #endif
    }
    else
    {
      #ifdef DEBUG
        log::toFile("Failed to create DyrBot with UUID{%}", id);
      #endif
    }

    return id;
  }

  /*Returns true if the bot was deleted
    False otherwise*/
  bool BotManager::deleteBot(int id)
  {
    bool success = false;

    //Check if key exist
    auto id_bot_map_iter = id_bot_map.find(id);

    if( id_bot_map_iter != id_bot_map.end() )
    {
      id_bot_map.erase(id_bot_map_iter);

      success = true;

      #ifdef DEBUG
        log::toFile("DyrBot with UUID{%} deleted", id);
      #endif
    }
    else
    {
      #ifdef DEBUG
        log::toFile("Failed to delete DyrBot with UUID{%}", id);
      #endif
    }

    return success;
  }
}
