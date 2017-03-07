#ifndef DYRBOT_HPP
#define DYRBOT_HPP

#include <functional>
#include <string>
#include <random>
#include <mutex>
#include <queue>
#include <map>
#include <set>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/system/error_code.hpp>
#include <boost/chrono.hpp>

#include "DyrBotErrors.hpp"
#include "message_struct.hpp"
#include "privmsg_struct.hpp"

namespace dyr
{
 class BotManager;

 namespace asio   = boost::asio;
 namespace ip     = asio::ip;
 namespace system = boost::system;

 typedef boost::chrono::high_resolution_clock high_res_clock;

 class DyrBot
 {
     public:
         typedef std::function<void(DyrBot*, const irc_message_struct*, irc_privmsg_struct*)> privmsg_function;
         typedef void privmsg_func_sig(const irc_message_struct*, irc_privmsg_struct*);
         typedef void(*privmsg_func_ptr)(DyrBot*, const irc_message_struct*, irc_privmsg_struct*);
         //Construct bot using config filename
         DyrBot(
             const int id,
             BotManager& bot_manager_shared,
             std::string config_filename  = ""
         );

         //Request to connect to server
         void request_connect_to_server();

         void request_disconnect(const irc_message_struct* irc_message = nullptr, irc_privmsg_struct* irc_privmsg = nullptr);

         //Loop for continually making send and receive request
         void message_pump();

         //Closes the connection immediately
         void self_destruct();

     private:
         //Initialize status variables
         void initialize_status();
         
         //Map strings to privmsg functions
         void initialize_privmsg_commands();
         
         //Load config file
         bool load_config();

         //Load hardcoded default settings
         void load_default_settings();

         //Callback for request_connect_to_server
         void connect_handler(
          const system::error_code& ec,
          asio::ip::tcp::resolver::iterator iter
         );

         //Request to send message asynchronously to server
         void request_send(const std::string& message);
         void request_send(std::string&& message);

         //Callback for request_send
         void send_handler(
          const system::error_code& ec,
          std::size_t bytes_sent
         );

         //Request to receive message asynchronously from server
         void request_receive();

         //Callback for request_send
         void receive_handler(
          const boost::system::error_code& ec,
          std::size_t bytes_received
         );

         void message_handler();
         void log_error(const boost::system::error_code& ec);

         void register_connection();
         void join(const std::string& channel);
         void part(const std::string& channel);
         void part_all(const irc_message_struct* irc_message = nullptr, irc_privmsg_struct* irc_privmsg = nullptr);
         void privmsg(std::string target, std::string message);

         void change_nick();
         void change_nick(std::string nickname);
         void change_nick(const irc_message_struct* irc_message, irc_privmsg_struct* irc_privmsg);
         void privmsg_handler(const irc_message_struct& message);
         
         void meta_command(const irc_message_struct* irc_message, irc_privmsg_struct* irc_privmsg);

         void disconnect(const irc_message_struct* irc_message = nullptr, irc_privmsg_struct* irc_privmsg = nullptr);
            
         void notify_manager_ready();
         void notify_manager(DyrError&& error);
         
         void substitute_variables(std::string& str);

         int bot_id;

         int pending_receives;
         int pending_sends;
         std::string partial_string;

         BotManager& bot_manager;

         std::queue<boost::array<char, 512> > recbuf;
         std::vector<std::string> unparsed_messages;
         std::queue<irc_message_struct> messages;

         std::set<std::string> channels;

         ip::tcp::socket tcp_socket;

         std::map<std::string, privmsg_function> command;
         std::map<std::string, std::string> setting;
         std::map<std::string, bool> status;

         std::default_random_engine rng;
         high_res_clock::time_point begin_time;
         high_res_clock::time_point end_time;
         high_res_clock::duration   time_to_connect;
 };
}

#endif /*DYRBOT_HPP*/
