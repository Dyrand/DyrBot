#ifndef BOT_FACTORY_HPP
#define BOT_FACTORY_HPP

namespace dyr
{
 class BotFactory
 {
     public:
        BotFactory(std::string default_config = std::string("config/config.txt"));
        
        template<typename Bot>
        Bot createBot(std::string config_file)
        {
            Bot fledgeling(config_file);
            return fledgeling;
        }
        
        template<typename Bot>
        Bot createBot()
        {
            return createBot<Bot>(default_config_file);
        }
     
     private:
        std::string default_config_file;
 };
}

#endif

