#include <iostream>
#include <utility>
#include <string>
#include <chrono>
#include <thread>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>
#include <boost/make_shared.hpp>

#include "ConnectionManager.hpp"
#include "DyrBot.hpp"
#include "Logger.hpp"

namespace dyr
{
  namespace asio  = boost::asio;
  namespace ip    = asio::ip;
  namespace system = boost::system;

  //Static member initialization
  asio::io_service ConnectionManager::io_service;
  ip::tcp::resolver ConnectionManager::resolver(ConnectionManager::io_service);

  ip::tcp::resolver::iterator ConnectionManager::resolve(
    const std::string& hostname,
    const int& port
  )
  {
    system::error_code ec;

    asio::ip::tcp::resolver::query tcp_query(hostname, std::to_string(port));
    auto endpoint_iterator = resolver.resolve(std::move(tcp_query), ec);

    if(ec)
    {
      #ifdef DEBUG
        log::toFile(ec.message());
      #endif
    }

    return endpoint_iterator;
  }

  ip::tcp::resolver::iterator ConnectionManager::resolve(
    const std::string& hostname,
    const std::string& port
  )
  {
    return resolve(hostname, std::stoi(port));
  }

  asio::io_service& ConnectionManager::get_io_service()
  { return io_service; }

  void ConnectionManager::process()
  {
    system::error_code ec;

    while(!ec)
    {
      io_service.poll(ec);
      io_service.reset();

      boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
    }

    if(ec)
    {
      #ifdef DEBUG
        log::toFile(ec.message());
      #endif
    }
  }
}
