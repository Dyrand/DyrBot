#ifndef BOT_MANAGER_HPP
#define BOT_MANAGER_HPP

#include <queue>
#include <mutex>
#include <map>

#include <boost/thread.hpp>

#include "DyrBot.hpp"
#include "DyrBotErrors.hpp"

namespace dyr
{
    class BotManager
    {
        public:
            BotManager(){}
            BotManager(const std::string& default_config);
            ~BotManager();

            int getBotCount();

			/*Returns the id of the bot created
			   -1 otherwise */
            int createBot(std::string config_file = "");
            bool deleteBot(int id);
            bool exist(int id);

            void connectBots(int delay);
            void process_loop();

            void notify_ready(const int& id);
            void notify_error(const int& id, DyrError&& error); 

        private:
            std::string default_config_file;
            
            boost::mutex mtx;

            std::map<int, DyrBot> id_bot_map;
            std::map<int, boost::thread> id_bot_thread;

            int generateID();

            BotManager(const BotManager&) = delete; //Copy Constructor
            BotManager(BotManager&& other) noexcept = delete; //Move Constructor
            BotManager& operator=(const BotManager& other) = delete; //Copy Assignment
            BotManager& operator=(BotManager&& other) noexcept = delete; //Move Assignment
    };
}

#endif /*BOT_MANAGER_HPP*/
