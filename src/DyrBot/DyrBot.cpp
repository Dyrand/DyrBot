#include <exception>
#include <iostream>
#include <fstream>
#include <random>
#include <memory>
#include <vector>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/system/error_code.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include "ConnectionManager.hpp"
#include "BotManager.hpp"
#include "Logger.hpp"
#include "DyrBot.hpp"
#include "DyrBotErrors.hpp"
#include "Parser.hpp"
#include "message_struct.hpp"
#include "privmsg_struct.hpp"

namespace dyr
{
 namespace asio = boost::asio;
 namespace ip =   asio::ip;
 namespace system = boost::system;
 namespace placeholders = asio::placeholders;

 typedef boost::chrono::high_resolution_clock high_res_clock;

 //Construct bot using config filename
 DyrBot::DyrBot(
  int id,
  BotManager& bot_manager_shared,
  std::string config_filename
 ):
 bot_id(id),
 bot_manager(bot_manager_shared),
 tcp_socket(ConnectionManager::get_io_service())
 {
     setting["config_filename"] = config_filename;
     initialize_status();
      
     if(!load_config())
     { load_default_settings(); }
 }


 //Initialize status variables
 void DyrBot::initialize_status()
 {
     status["config_loaded"] = false;
     status["connected_to_server"] = false;
     status["ready_to_send"] = false;
     status["ready_to_receive"] = false;
     status["request_to_disconnect"] = false;
     pending_receives = 0;
     pending_sends = 0;
 }

 //Load config file
 bool DyrBot::load_config()
 {
     std::ifstream config_in(setting["config_filename"]);

     if(config_in.fail())
     {
         log::toFile("In DyrBot::load_config for bot {%}", bot_id);
         log::toFile("Failed to load config file: %", setting["config_filename"]);

         notify_manager(DyrError::load_config);
         return false;
     }

     std::string line, key, value;

     config_in >> line;

     while(!config_in.fail())
     {
         auto pos = line.find('=');
         key = line.substr(0,pos);
         value = line.substr(pos+1);
         setting[key] = value;
         config_in >> line;
     }

     if(!config_in.eof())
     {
         log::toFile("In DyrBot::load_config for bot {%}", bot_id);
         log::toFile("Failed to load config file: %", setting["config_filename"]);
         
         notify_manager(DyrError::load_config);
         return false;
     }

     status["config_loaded"] = true;
     return true;
 }
 
 void DyrBot::load_default_settings()
 {
     setting["hostname"]            = "irc.freenode.com";
     setting["port"]                = "6667";
     setting["connection_password"] = "password";
     setting["default_channels"]    = "#dyrbot";
     setting["nickname"]            = "DyrBot";
     setting["username"]            = "DyrBot";
     setting["realname"]            = "DyrBot";
     setting["mode"]                = "0";
     setting["command_ident"]       = ">";
 }

 //Request to connect to server
 bool DyrBot::request_connect_to_server()
 {
     system::error_code ec;

     const ip::tcp::resolver::iterator endpoint_iterator =
      ConnectionManager::resolve(setting["hostname"], setting["port"]);

     if(ec)
     {
         log::toFile("In DyrBot::request_connect_to_server");
         log::toFile(ec.message());

         error_queue.push(std::move(ec));
         notify_manager(DyrError::request_connect_to_server);
         return false;
     }

     //Binded connect_handler
     auto connection_handler =
      boost::bind(
       &DyrBot::connect_handler,
       this,
       placeholders::error,
       placeholders::iterator
      );

      asio::async_connect(tcp_socket, endpoint_iterator, connection_handler);
      begin_time = high_res_clock::now();

      return true;
 }

 //Callback for request_connect_to_server
 void DyrBot::connect_handler(
  const system::error_code& error,
  asio::ip::tcp::resolver::iterator iter
 )
 {
     if(error)
     {
         log::toFile("DyrBot::connect_handler");
         log::toFile(error.message());

         error_queue.push(std::move(error));
         notify_manager(DyrError::connect_handler);
     }
     else
     {
         status["connected_to_server"] = true;
         status["ready_to_send"] = true;
         status["ready_to_receive"] = true;

         end_time = high_res_clock::now();
         time_to_connect = end_time - begin_time;
         rng.seed(time_to_connect.count());

         notify_manager_ready();
         register_connection();
     }
 }

 //Request to send message asynchronously to server
 void DyrBot::request_send(const std::string& message)
 {
     if(!status["ready_to_send"])
     {return;}

     std::cout << "(SENDING):" << message << std::endl;

     auto binded_send_handler =
      boost::bind(
       &DyrBot::send_handler,
       this,
       placeholders::error,
       placeholders::bytes_transferred
      );

     //Shared ptr to extend lifetime of sendbuf beyond scope
     std::shared_ptr<std::vector<char> > sendbuf =
     std::make_shared<std::vector<char> >(message.begin(),message.end());

     sendbuf->push_back('\r');
     sendbuf->push_back('\n');

     tcp_socket.async_send(asio::buffer(*sendbuf), binded_send_handler);
     ++pending_sends;
 }

 //Request to send message asynchronously to server
 void DyrBot::request_send(std::string&& message)
 {
     if(!status["ready_to_send"])
     {return;}

     std::cout << "(SENDING):" << message << std::endl;

     auto binded_send_handler =
      boost::bind(
       &DyrBot::send_handler,
       this,
       placeholders::error,
       placeholders::bytes_transferred
      );

      message += "\r\n";

      //Shared ptr to extend lifetime of sendbuf beyond scope
      std::shared_ptr<std::vector<char> > sendbuf =
      std::make_shared<std::vector<char> >(message.begin(), message.end());

      tcp_socket.async_send(asio::buffer(*sendbuf), binded_send_handler);
      ++pending_sends;
 }

 //Callback for request_send
 void DyrBot::send_handler(
  const system::error_code& ec,
  std::size_t bytes_sent
 )
 {
     --pending_sends;
     if(ec)
     {
         log::toFile("In DyrBot::send_handler");
         log::toFile(ec.message());

         error_queue.push(std::move(ec));
         notify_manager(DyrError::send_handler);
     }
 }

 //Request to receive message asynchronously from server
 void DyrBot::request_receive()
 {
     if(!status["ready_to_receive"])
     {return;}

     auto binded_receive_handler =
      boost::bind(
       &DyrBot::receive_handler,
       this,
       placeholders::error,
       placeholders::bytes_transferred
      );

      recbuf.emplace();
      tcp_socket.async_receive(asio::buffer(recbuf.back()), binded_receive_handler);
      ++pending_receives;
 }

 //Callback for request_send
 void DyrBot::receive_handler(
  const boost::system::error_code& ec,
  std::size_t bytes_received
 )
 {
     static std::string partial_string;

     --pending_receives;
     if(ec)
     {
         log::toFile("In DyrBot::receive_handler");
         log::toFile(ec.message());

         error_queue.push(std::move(ec));
         notify_manager(DyrError::receive_handler);
     }
     else
     {
         std::string recv_string(
          recbuf.front().begin(),
          recbuf.front().begin()+bytes_received
         );

         //Check if entire recv_string is partial
         if(recv_string.find("\r\n") == std::string::npos)
         {
             partial_string.append(recv_string);
             return;
         }

         partial_string =
          parse::raw_message(partial_string + recv_string, unparsed_messages);

         for(std::string text : unparsed_messages)
         {
             std::cout << text << std::endl;
             messages.push(parse::irc_message(text));
         }
     }

     recbuf.pop();
 }

 //Loop for continually making send and receive request
 void DyrBot::message_pump()
 {
     while(status["connected_to_server"])
     {
         if(pending_receives <= 5 && !status["request_to_disconnect"])
        { request_receive(); }

        message_handler();
        error_handler();

        boost::this_thread::sleep_for(boost::chrono::milliseconds(16));

        if(status["request_to_disconnect"] &&
          (pending_receives == 0) &&
          (pending_sends == 0))
        { disconnect(); }
     }

     log::toFile("Bot {%} message_pump ended", bot_id);
 }

 void DyrBot::message_handler()
 {
     while(!messages.empty())
     {
         irc_message_struct message = messages.front();

        if(message.command == "PING")
        { request_send("PONG " + message.parameters); }
        else if(message.command == "NOTICE")
        { /*notify_subs();*/ }
        else if(message.command == "PRIVMSG")
        { /*privmsg_handler(message);*/ }
        else if(message.command == "001") //RPL_WELCOME
        { join(setting["default_channels"]); }
        else if(message.command == "332") //RPL_TOPIC
        {  }
        else if(message.command == "432") //Erronoeous Nickname
        { change_nick(); }
        else if(message.command == "433") //ERR_NICKNAMEINUSE
        { change_nick(); }
        else if(message.command == "436") //ERR_NICKCOLLISION
        { change_nick(); }
        
        messages.pop();
    }
 }

 void DyrBot::error_handler()
 {
    //Needs real implementation
    while(!error_queue.empty())
    { error_queue.pop(); }
 }

 void DyrBot::register_connection()
 {
     request_send("PASS " + setting["connection_password"]);
     request_send("NICK " + setting["nickname"]);
     request_send("USER " + setting["username"] + " " + setting["mode"] + " * " + " :" + setting["realname"]);
 }

 void DyrBot::join(const std::string& channel)
 {
     channels.emplace_back(channel);
     request_send("JOIN " + channel);
 }

 void DyrBot::part(const std::string& channel)
 {
     request_send("PART " + channel);
 }

 void DyrBot::part_all()
 {
     for(auto& channel: channels)
     { part(channel); }
 }

 void DyrBot::change_nick()
 {
     static std::string nick("NICK ");
     
     std::string nickname;
     nickname += (rng()%76)+48;
     for(int i(1); i <= 8; ++i)
     { nickname += (rng()%76)+48; }

     request_send(nick + nickname);
 }

 void DyrBot::request_disconnect()
 {
     part_all();

     status["request_to_disconnect"] = true;
     status["ready_to_send"] = false;
     status["ready_to_receive"] = false;
 }

 void DyrBot::disconnect()
 {
     boost::system::error_code ec;
     tcp_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
     tcp_socket.close();
     status["connected_to_server"] = false;
 }

 bool DyrBot::notify_manager_ready()
 {
     bot_manager.notify_ready(bot_id);
     return true;
 }

 void DyrBot::notify_manager(DyrError&& error)
 {
     bot_manager.append_error(bot_id, std::move(error));
 }
}
