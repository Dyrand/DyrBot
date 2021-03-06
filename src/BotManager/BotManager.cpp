#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <tuple>
#include <map>

#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "Identifier.hpp"
#include "Logger.hpp"
#include "BotManager.hpp"
#include "ConnectionManager.hpp"
#include "DyrBot.hpp"
#include "DyrBotErrors.hpp"

namespace dyr
{
 BotManager::BotManager(std::string default_config)
 {
     default_config_file = default_config;
 }
 
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

 /*Returns the id of the bot created
  -1 otherwise */
 int BotManager::createBot(std::string config_file)
 {
     if(config_file == "")
     { config_file = default_config_file; }
     
     int bot_id = id::generate();

     if( bot_id != -1 )
     {
         id_bot_map.emplace(
          std::piecewise_construct,
          std::forward_as_tuple(bot_id),
          std::forward_as_tuple(bot_id, *this, config_file));

         log::toFile("Created DyrBot with ID {%}", bot_id);
     }
     else
     {
         log::toFile("Failed to create DyrBot with ID {%}", bot_id);
     }

     return bot_id;
 }

 /*Returns true if the bot was deleted
   false otherwise*/
 bool BotManager::deleteBot(int id)
 {
     bool success = false;

     if( exist(id) )
     {
         id_bot_map.at(id).self_destruct();
         success = true;

         log::toFile("DyrBot with ID {%} deleted", id);
     }
     else
     {
         log::toFile("DyrBot with ID{%} does not exist, so it could not be deleted", id);
     }

     return success;
 }

 bool BotManager::exist(int bot_id)
 {
     bool bot_existance = false;

     auto id_bot_map_iter = id_bot_map.find(bot_id);

     if( id_bot_map_iter != id_bot_map.end() )
     { bot_existance = true; }

     return bot_existance;
 }

 void BotManager::connectBots(int delay)
 {
     for(auto& iter: id_bot_map)
     {
         int bot_id = iter.first;
         id_bot_thread.emplace(bot_id, boost::thread());
         iter.second.request_connect_to_server();
         
         boost::this_thread::sleep_for(boost::chrono::milliseconds(delay));
     }
 }

 //Called by bot to indicate to BotManager that it's ready for the message_pump thread
 void BotManager::notify_ready(const int& id)
 {
     id_bot_thread.at(id) = boost::thread(&DyrBot::message_pump, &id_bot_map.at(id));
     id_bot_thread.at(id).detach();
 }
 
 //Called by bot to indicate there was an error
 void BotManager::notify_error(const int& id, DyrError&& error)
 {
     if(error == DyrError::disconnected)
     {
        id_bot_map.at(id).request_connect_to_server();
     }
 }
}
