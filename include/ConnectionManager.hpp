#ifndef CONNECTION_MANAGER_HPP
#define CONNECTION_MANAGER_HPP

#include <thread>
#include <vector>
#include <utility>
#include <memory>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include "DyrBot.hpp"

namespace dyr
{
  namespace asio = boost::asio;
  namespace ip   = asio::ip;

  class ConnectionManager
  {
    public:

      static ip::tcp::resolver::iterator resolve(
        const std::string& hostname,
        const int& port
      );

      static ip::tcp::resolver::iterator resolve(
        const std::string& hostname,
        const std::string& port
      );

      static asio::io_service& get_io_service();

      static void process();

    private:
      ConnectionManager();

      /*Socket related*/
      static asio::io_service io_service;
      static ip::tcp::resolver resolver;
  };
}

#endif /*CONNECTION_MANAGER_HPP*/
