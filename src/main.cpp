#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <map>

#include "BotManager.hpp"

int main(int argc, char *argv[])
{
  auto printHelp = [](){
    std::cout << "DyrBot is a multiplexed bot program\n\n"
    "Usage:\n"
    "\tdyrbot [-number_of_bots | -n <number>]"
    "\n\n"
    "Options:\n"
    "\t-h --help      Show this screen\n"
    "\t-n             Set the number of bots to create\n";

    return 1;
  };

  int number_of_bots = 1;

  std::string arg;
  for(int index = 1; index < argc; ++index)
  {
    arg = argv[index];
    if(arg == "-number_of_bots" || arg == "-n")
    {
      //Get all arguments until next modifier is reached
      bool valid = false;
      while(index+1 < argc)
      {
        if(argv[index+1][0] == '-')
        {
          if(!valid)
          { return printHelp(); }
          break;
        }
        else
        {
          ++index;
          arg = argv[index];
          valid = true;
        }
      }
      if(valid)
      {
        try
        {
          number_of_bots = std::stoi(arg);
        }
        catch(const std:: exception& e)
        {
          std::cout << "-number_of_bots requires a number between 0 and "
                    << std::numeric_limits<int>::max();
        }
      }
    }
    else if(arg == "-h" || arg == "--help")
    {
      return printHelp();
    }
    else
    {
      return printHelp();
    }
  }

  dyr::BotManager manager;
  std::vector<int> ids;

  for(int count = 1; count <= number_of_bots; ++count)
  { ids.emplace_back( manager.createBot() ); }

  for(int& id : ids)
  { manager.deleteBot(id); }
}
