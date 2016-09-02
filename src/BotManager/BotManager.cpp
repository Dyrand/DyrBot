#include <iostream>
#include <utility>
#include <vector>
#include <thread>
#include <chrono>
#include <tuple>
#include <map>

#include <boost/chrono.hpp>

#include "UniqueIdentifier.hpp"
#include "Logger.hpp"
#include "BotManager.hpp"
#include "ConnectionManager.hpp"
#include "DyrBot.hpp"

namespace dyr
{
  BotManager::~BotManager()
  {
      //Disconnect all bots connected
      for(auto &iter : id_bot_map)
      { iter.second.request_disconnect(); }
  }

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
        std::forward_as_tuple("config/config.txt")
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
    false otherwise*/
  bool BotManager::deleteBot(int id)
  {
    bool success = false;

    if( exist(id) )
    {
      id_bot_map.at(id).request_disconnect();

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

  bool BotManager::exist(int id)
  {
    bool bot_existance = false;

    auto id_bot_map_iter = id_bot_map.find(id);

    if( id_bot_map_iter != id_bot_map.end() )
    { bot_existance = true; }

    return bot_existance;
  }

  void BotManager::process_loop()
  {
    for(auto& iter: id_bot_map)
    {
      int uuid = iter.first;
      id_bot_thread.emplace(uuid, boost::thread());
      iter.second.request_connect_to_server();
    }

    for(auto& iter: id_bot_thread)
    {
      iter.second = boost::thread(&DyrBot::message_pump, &id_bot_map.at(iter.first));
      iter.second.detach();
      boost::this_thread::sleep_for(boost::chrono::seconds(5));
    }
  }
}
