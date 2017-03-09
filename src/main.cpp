#include <exception>
#include <iostream>
#include <limits>
#include <thread>
#include <string>
#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/program_options.hpp>

#include "ConnectionManager.hpp"
#include "BotManager.hpp"
#include "Parser.hpp"

namespace po = boost::program_options;

po::options_description getCommandLineOptions()
{
    po::options_description desc("DyrBot Options");
    desc.add_options()
        ("help", "display this help screen")
        ("number_of_bots,n", po::value<int>()->default_value(1), "set number of bots to create")
        ("connection_delay,c", po::value<int>()->default_value(1000), "set the number of milliseconds to wait before connecting another bot")
        ("default_config_file,d", po::value<std::string>()->default_value("config/config.txt"), "set the default config file for bots to use");
    return desc;
}

int main(int argc, char *argv[])
{    
    po::options_description desc = getCommandLineOptions();
    po::variables_map vm;
    
    try
    {
        vm = dyr::parse::command_line(argc, argv, desc);
    }
    catch(const std::exception& e)
    {
        std::cout << "Invalid input arguments\n" << std::endl;
        return 2;
    }
    
    if(vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }
    
    int number_of_bots = vm["number_of_bots"].as<int>();
    int delay = vm["connection_delay"].as<int>();
    std::string default_config_file = vm["default_config_file"].as<std::string>();
    
    dyr::BotManager bot_manager(default_config_file);

    boost::thread lp(dyr::ConnectionManager::process);
    
    for(int count = 1; count <= number_of_bots; ++count)
    { bot_manager.createBot(); }

    bot_manager.connectBots(delay);
    lp.join();
}
