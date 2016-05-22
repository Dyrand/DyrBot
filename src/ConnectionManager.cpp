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
  namespace system = boost::system;
  namespace asio  = boost::asio;

  ConnectionManager::ConnectionManager(bool debug_mode_):
    resolver(io_service),
    debug_mode(debug_mode_)
  {}

  void ConnectionManager::connect(
    std::string&& hostname,
    std::string&& port,
    std::string&& config_file,
    std::string&& id_number)
  {
    system::error_code ec;

    asio::ip::tcp::resolver::query tcp_query(hostname, port);
    auto endpoint_iterator = resolver.resolve(std::move(tcp_query), ec);
    if(ec && debug_mode)
    { logError::toFile(ec.message()); }
    else
    {
       //Construct a DyrBot
      initialized_bots.emplace_back(new DyrBot(
        io_service, std::move(config_file), std::move(id_number)));
       //Attempt a connection
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
            std::thread(std::bind(active_bots.back().first->process,active_bots.back().first.get()));
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if(ec)
    { logError::toFile(ec.message()); }
  }
}
