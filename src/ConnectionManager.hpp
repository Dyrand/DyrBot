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

  class ConnectionManager
  {
    public:
      ConnectionManager(bool debug_mode = false);

      bool instantiate(
        std::string&& hostname,
        int&& port,
        std::string&& config_file = ""
      );
      void process();

    private:
      /*Socket related*/
      asio::io_service io_service;
      asio::ip::tcp::resolver resolver;
      std::vector<boost::shared_ptr<DyrBot> > initialized_bots;
      std::vector<std::pair<boost::shared_ptr<DyrBot>,std::thread> > active_bots;

      bool debug_mode;
  };
}

#endif /*CONNECTION_MANAGER_HPP*/
