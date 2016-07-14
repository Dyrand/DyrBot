#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <utility>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include "ConnectionManager.hpp"
#include "DyrBot.hpp"
#include "error/error_logger.hpp"

namespace dyr
{
  namespace asio  = boost::asio;
  namespace system = boost::system;

  ConnectionManager::ConnectionManager(bool debug_mode_):
    resolver(io_service),
    debug_mode(debug_mode_)
  {}

  bool ConnectionManager::instantiate(
    std::string&& hostname,
    int&& port,
    std::string&& config_file)
  {
    system::error_code ec;

    asio::ip::tcp::resolver::query tcp_query(hostname, std::to_string(port));
    auto endpoint_iterator = resolver.resolve(std::move(tcp_query), ec);
    if(ec)
    {
      if(debug_mode)
      {
        logError::toFile(ec.message());
        logError::toFile("Failed to instantiate bot with config file: \""+config_file+"\"");
      }
      else
      {
        std::cerr << "Failed to instantiate bot with config file: \"" << config_file << "\"" << std::endl;
      }

      return false;
    }
    else
    {
      //Construct a DyrBot
      initialized_bots.emplace_back(
        new DyrBot(
          io_service,
          std::move(config_file)));
      //Queue a connection
      initialized_bots.back()->connect(endpoint_iterator);
    }
  }

  void ConnectionManager::process()
  {
    system::error_code ec;

    while(!ec)
    {
      io_service.poll(ec);
      io_service.reset();

      for(int index(0); index < initialized_bots.size(); ++index)
      {
        //Check what initialized bots are ready
        if(initialized_bots.at(index)->ready())
        {
          //Move Bot into active_bot vector
          active_bots.emplace_back(std::move(initialized_bots.at(index)), std::thread());
          //Erase old bot from initialized_bots
          initialized_bots.erase(initialized_bots.begin()+index);
          //Create actual thread for pair
          active_bots.back().second =
            std::thread(&DyrBot::process,active_bots.back().first.get());
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      
      //Check if any bots still exist
      if(active_bots.size() == 0 && initialized_bots.size() == 0)
      { break; }
    }
    if(ec)
    { logError::toFile(ec.message()); }
  }
}
