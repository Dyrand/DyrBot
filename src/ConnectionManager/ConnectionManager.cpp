#include <string>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/system/error_code.hpp>

#include "ConnectionManager.hpp"
#include "Logger.hpp"

namespace dyr
{
 namespace asio   = boost::asio;
 namespace ip     = asio::ip;
 namespace system = boost::system;

 //Static member initialization
 asio::io_service ConnectionManager::io_service;
 ip::tcp::resolver ConnectionManager::resolver(ConnectionManager::io_service);

 ip::tcp::resolver::iterator ConnectionManager::resolve(
  const std::string& hostname,
  const std::string& port
 )
 {
     system::error_code ec;

     asio::ip::tcp::resolver::query tcp_query(hostname, port);
     auto endpoint_iterator = resolver.resolve(std::move(tcp_query), ec);

     if(ec)
     {
         log::toFile("In ConnectionManager::resolve");
         log::toFile(ec.message());
     }

     return endpoint_iterator;
 }

 asio::io_service& ConnectionManager::get_io_service()
 { return io_service; }

 void ConnectionManager::process()
 {
     system::error_code ec;

     while(!ec)
     {
         io_service.run(ec);
         io_service.reset();
         
         boost::this_thread::sleep_for(boost::chrono::milliseconds(10));
     }

     if(ec)
     {
         log::toFile("In ConnectionManager::process");
         log::toFile(ec.message());
     }
 }
}
