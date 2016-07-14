#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "ConnectionManager.hpp"
#include "modules/module.hpp"

int main(int argc, char *argv[])
{
  std::string help_message =
   "Usage: DyrBot [-default_server|-ds <IP|hostname>] "
   "[-default_port|-dp <number>]\n"
   "\t\t[-config|-c <path>] "
   "[-server|-s <IP|hostname>]\n"
   "\t\t[-port|-p <number>] "
   "[-minimum_number_of_bots|-m <number>]\n";

  std::string default_config = "config.txt";
  std::string default_server = "irc.freenode.com";
  int default_port = 8001;

  int bot_count = 0;
  bool debug_mode = false;

  std::vector<std::string> config;
  std::vector<std::string> server;
  std::vector<int> port;

  auto help = [&help_message]()
  {
    std::cout << help_message;
    return 1;
  };

  if(argc == 1)
  { return help(); }

  for(int i(1); i < argc; ++i)
  {
    std::string arg_str(argv[i]);
    if(arg_str == "--help")
    { return help(); }
    else if(arg_str == "-default_server" || arg_str == "-ds")
    {
      ++i;
      if(i < argc)
      {
        arg_str = argv[i];
        default_server = std::move(arg_str);
      }
      else
      { return help(); }
    }
    else if(arg_str == "-config" || arg_str == "-c")
    {
      ++i;
      if(i < argc)
      {
        arg_str = argv[i];
        config.emplace_back(std::move(arg_str));
      }
      else
      { return help(); }
    }
    else if(arg_str == "-server" || arg_str == "-s")
    {
      ++i;
      if(i < argc)
      {
        arg_str = argv[i];
        server.emplace_back(std::move(arg_str));
      }
      else
      { return help(); }
    }
    else if(arg_str == "-port" || arg_str == "-p")
    {
      ++i;
      if(i < argc)
      {
        arg_str = argv[i];
        try
        {
          int temp_int = std::stoi(arg_str);
          if(temp_int > 1 && temp_int < 65535)
          { port.emplace_back(std::move(temp_int)); }
          else
          { throw(std::out_of_range("out of range")); }
        }
        catch(const std::exception& e)
        {
          std::cout << "-port takes a number from 1 to 65535\n";
          return 1;
        }
      }
      else
      { return help(); }
    }
    else if(arg_str == "-minimum_number_of_bots" || arg_str == "-m" )
    {
      ++i;
      if(i < argc)
      {
        try
        {
          arg_str = argv[i];
          bot_count = std::stoi(arg_str);
          if(bot_count < 0)
          { throw(std::out_of_range("out of range")); }
        }
        catch(const std::exception& e)
        {
          std::cout << "-minimum_number_of_bots takes a number from 1 to "
                    << std::numeric_limits<int>::max() << "\n";
        }
      }
      else
      { return help(); }
    }
    else if(arg_str == "--debug")
    { debug_mode = true; }
    else
    { return help(); }
  }

  //Check for highest number of arguments in one type of parameter
  int temp_count = 0;
  if(config.size() > server.size())
  { temp_count = config.size(); }
  else
  { temp_count = server.size(); }
  if(port.size() > bot_count)
  { temp_count = port.size(); }
  if(temp_count > bot_count)
  { bot_count = temp_count; }

  //Fill in the blanks with defaults
  for(int i(config.size()); i < bot_count; ++i)
  { config.emplace_back(default_config); }
  for(int i(server.size()); i < bot_count; ++i)
  { server.emplace_back(default_server); }
  for(int i(port.size()); i < bot_count; ++i)
  { port.emplace_back(default_port); }

  //Initialize modules
  boost::shared_ptr<dyr::module::ModuleManager> module_manager(dyr::module::ModuleManager::get());
  module_manager.get()->InitializeModules();

  dyr::ConnectionManager manager(debug_mode);
  for(int i(0); i < bot_count; ++i)
  {
    manager.instantiate(
      std::move(server.at(i)),
      std::move(port.at(i)),
      std::move(config.at(i)));
  }

  manager.process();

  return 0;
}
