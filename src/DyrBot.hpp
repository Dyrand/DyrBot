#ifndef DYRBOT_HPP
#define DYRBOT_HPP

#include <array>
#include <queue>
#include <map>
#include <ctime>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/system/error_code.hpp>

#include "structs/message_struct.hpp"
#include "structs/privmsg_struct.hpp"

namespace dyr
{
  namespace asio = boost::asio;
  namespace filesystem = boost::filesystem;

  class DyrBot: public boost::enable_shared_from_this<DyrBot>
  {
    public:
      DyrBot(
        asio::io_service& io_service_,
        std::string&& config_file_path = "",
        std::string&& id_number = "-1");

      void connect(asio::ip::tcp::resolver::iterator& endpoint_iterator);
      void process();
      bool ready();

    private:
      void configure();
      void save_config();
      void generate_config_file(const filesystem::path& config_path);
      void load_file();

      void register_connection();

      void changeNick(const std::string& nick);
      void join(const std::string& channel);
      void privmsg(const std::string& target, const std::string& message);

      void send(std::string&& text);
      void send(const std::string& text);
      void receive();

      void send_handler(
        const boost::system::error_code& ec, // Result of operation.
        std::size_t bytes_sent               // Number of bytes sent.
      );

      void receive_handler(
        const boost::system::error_code& ec, // Result of operation.
        std::size_t bytes_received           // Number of bytes received.
      );

      void connect_handler(
        const boost::system::error_code& ec,    // Result of operation.
        asio::ip::tcp::resolver::iterator iter  // Iterator to endpoint
      );

      void message_handler();
      void privmsg_handler(const Message_Struct& message);

      asio::ip::tcp::socket tcp_socket;
      std::map<std::string,std::string> var;
      filesystem::path config_file;

      std::string partial_string;
      std::string recv_string;
      std::queue<std::array<char, 512> > recbuf;
      std::queue<Message_Struct> msg_queue;
      Message_Struct message;
      std::queue<std::string> test_nicks;

      bool stay_connected;
      bool ready_to_process;
      int pending_receives;
      int id_number;

      std::default_random_engine random_num;
      std::chrono::high_resolution_clock::time_point begin_time;
      std::chrono::high_resolution_clock::time_point end_time;
      std::chrono::high_resolution_clock::duration time_to_connect;
  };
}

#endif /*DYRBOT_HPP*/
