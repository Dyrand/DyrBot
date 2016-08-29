#ifndef BOT_MANAGER_HPP
#define BOT_MANAGER_HPP

#include <map>

#include "DyrBot.hpp"

namespace dyr
{
  class BotManager
  {
    public:
      BotManager(){}
      //BotManager(const std::string& default_config);
      ~BotManager();

      int getBotCount();

      int createBot();
      bool deleteBot(int id);
      bool exist(int id);

      void process_loop();

    private:
      std::map<int, DyrBot> id_bot_map;
      std::map<int, std::thread> id_bot_thread;

      int generateID();

      BotManager(const BotManager&) = delete; //Copy Constructor
      BotManager(BotManager&& other) noexcept = delete; //Move Constructor
      BotManager& operator=(const BotManager& other) = delete; //Copy Assignment
      BotManager& operator=(BotManager&& other) noexcept = delete; //Move Assignment

  };
}

#endif /*BOT_MANAGER_HPP*/
