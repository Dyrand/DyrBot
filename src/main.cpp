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

po::options_description initialize_command_line_options()
{
    po::options_description desc("DyrBot Options:");
    desc.add_options()
        ("help", "display this help screen")
        ("number_of_bots,n", po::value<int>()->default_value(1), "set number of bots to create");
    return desc;
}

int main(int argc, char *argv[])
{    
    po::options_description desc = initialize_command_line_options();
    po::variables_map vm;
    
    try
    {
        vm = dyr::parse::command_line(argc, argv, desc);
    }
    catch(const std::exception& e)
    {
        std::cout << "Invalid input arguments\n" << std::endl;
        return 1;
    }
    
    if(vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }
    
    int number_of_bots = vm["number_of_bots"].as<int>();

    dyr::BotManager bot_manager;

    /*if(default_config_flag)
    { bot_manager = dyr::BotManager(default_config); }
    else
    { bot_manager = dyr::BotManager(); }*/

    boost::thread lp(dyr::ConnectionManager::process);


    for(int count = 1; count <= number_of_bots; ++count)
    { bot_manager.createBot(); }

    bot_manager.connectBots();
    //bot_manager.process_loop();
    lp.join();
}
