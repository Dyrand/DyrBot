#ifndef BOT_MANAGER_HPP
#define BOT_MANAGER_HPP

#include <map>

#include "DyrBot.hpp"

namespace dyr
{
  class BotManager
  {
    public:
      BotManager()  = default;
      ~BotManager() = default;

      int getBotCount();

      int createBot();
      bool deleteBot(int id);

    private:
      std::map<int, DyrBot> id_bot_map;

      int generateID();

      BotManager(const BotManager&) = delete; //Copy Constructor
      BotManager(BotManager&& other) noexcept = delete; //Move Constructor
      BotManager& operator=(const BotManager& other) = delete; //Copy Assignment
      BotManager& operator=(BotManager&& other) noexcept = delete; //Move Assignment

  };
}

#endif /*BOT_MANAGER_HPP*/
