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
#include <boost/algorithm/string.hpp>

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

 DyrBot::DyrBot(
  const int id,
  BotManager& bot_manager_shared,
  std::string config_filename
 ):
  bot_id(id),
  bot_manager(bot_manager_shared),
  tcp_socket(ConnectionManager::get_io_service())
 {
     setting["config_filename"] = config_filename;
     initialize_status();
     initialize_privmsg_commands();
      
     if(!load_config())
     { load_default_settings(); }
 }


 void DyrBot::initialize_status()
 {
     status["config_loaded"] = false;
     status["connected_to_server"] = false;
     status["ready_to_send"] = false;
     status["ready_to_receive"] = false;
     status["request_to_disconnect"] = false;
     status["self_destructed"] = false;
     pending_receives = 0;
     pending_sends = 0;
 }
 
 
 void DyrBot::initialize_privmsg_commands()
 {
     command["meta_command"] = &DyrBot::meta_command;
     command["part_all"] = &DyrBot::part_all;
     command["disconnect"] = &DyrBot::disconnect;
     command["request_disconnect"] = &DyrBot::request_disconnect;
     command["change_nick"] = std::mem_fn<privmsg_func_sig>(&DyrBot::change_nick);
     //command["change_setting"] = &change_settings;
 }

 
 bool DyrBot::load_config()
 {
     std::ifstream config_in(setting["config_filename"]);

     if(config_in.fail())
     {
         log::toFile("In DyrBot::load_config for bot {%}", bot_id);
         log::toFile("Failed to open config file: %", setting["config_filename"]);

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
         log::toFile("Failed to completely load config file: %", setting["config_filename"]);
         
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
     setting["substitutor"]         = "`";
     setting["retry_connect_wait"]  = "10000";
 }
 
 
 void DyrBot::self_destruct()
 {
     status["self_destructed"] = true;
     disconnect();
 }

 
 void DyrBot::request_connect_to_server()
 {
     if(status["self_destructed"])
     { return; }
 
     const ip::tcp::resolver::iterator endpoint_iterator =
      ConnectionManager::resolve(setting["hostname"], setting["port"]);

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
 }

 
 void DyrBot::connect_handler(
  const system::error_code& ec,
  asio::ip::tcp::resolver::iterator iter
 )
 {
     if(ec)
     {
         log::toFile("In DyrBot::connect_handler for Bot{%}", bot_id);
         log_error(ec);
         
         int delay;
         
         try
         {
            delay = std::stoi(setting["retry_connect_wait"]);
         }
         catch(const std::exception &e)
         {
             setting["reconnect_wait"] = "10000";
             delay = 10000;
         }
         
         status["connected_to_server"] = false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;

         boost::this_thread::sleep_for(boost::chrono::milliseconds(delay));
         
         notify_manager(DyrError::disconnected); 
     }
     else
     {
         log::toFile("Bot {%} connected succesfully", bot_id);
         
         status["connected_to_server"] = true;
         status["ready_to_send"] = true;
         status["ready_to_receive"] = true;
         
         end_time = high_res_clock::now();
         time_to_connect = end_time - begin_time;
         rng.seed(time_to_connect.count());
         
         register_connection();
         notify_manager_ready();
     }
 }

 
 void DyrBot::request_send(const std::string& message)
 {
     if(!status["ready_to_send"])
     { return; }

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

 
 void DyrBot::request_send(std::string&& message)
 {
     if(!status["ready_to_send"])
     { return; }

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

 
 void DyrBot::send_handler(
  const system::error_code& ec,
  std::size_t bytes_sent
 )
 {
     --pending_sends;
     
     if(ec)
     {
         log::toFile("In DyrBot::send_handler for Bot {%}", bot_id);
         log_error(ec);
     }
 }

 
 void DyrBot::request_receive()
 {
     if(!status["ready_to_receive"])
     { return; }

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

 
 void DyrBot::receive_handler(
  const boost::system::error_code& ec,
  std::size_t bytes_received
 )
 {
     --pending_receives;
     
     if(ec)
     {
         log::toFile("In DyrBot::receive_handler for Bot {%}", bot_id);
         log_error(ec);
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

         partial_string = parse::raw_message(partial_string + recv_string, unparsed_messages);

         for(std::string text : unparsed_messages)
         {
             std::cout << text << std::endl;
             substitute_variables(text);
             messages.push(parse::irc_message(text));
         }
     }

     recbuf.pop();
     
     request_receive();
 }

 
 void DyrBot::message_pump()
 {     
     //Start the chain of request_receive calls
     request_receive();
     
     while(status["connected_to_server"])
     {
         message_handler();

         if(status["request_to_disconnect"] && (pending_sends == 0))
         { disconnect(); }
     
         boost::this_thread::sleep_for(boost::chrono::milliseconds(20));
     }

     log::toFile("Bot {%} message_pump ended", bot_id);
     
     notify_manager(DyrError::disconnected);
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
         { privmsg_handler(message); }
         else if(message.command == "001") //RPL_WELCOME
         { join(setting["default_channels"]); }
         else if(message.command == "332") //RPL_TOPIC
         {  }
         else if(message.command == "432") //ERR_ERRONEUSNICKNAME
         { change_nick(); }
         else if(message.command == "433") //ERR_NICKNAMEINUSE
         { change_nick(); }
         else if(message.command == "436") //ERR_NICKCOLLISION
         { change_nick(); }
        
         messages.pop();
    }
 }

 
 void DyrBot::log_error(const boost::system::error_code& ec)
 {
     std::string error_value;
     auto error = ec.value();
     
     if( error == asio::error::access_denied)
     {
         error_value = " {access_denied}";
     }
     else if( error == asio::error::address_family_not_supported )
     {
            error_value = " {address_family_not_supported}";
     }
     else if( error == asio::error::address_in_use )
     {
            error_value = " {address_in_use}";
     }
     else if( error == asio::error::already_connected )
     {
        error_value = " {already_connected}";
     }
     else if( error == asio::error::already_started )
     {
            error_value = " {already_started}";
     }
     else if( error == asio::error::broken_pipe )
     {
         error_value = " {broken_pipe}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::connection_aborted )
     {
         error_value = " {connection_aborted}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::connection_refused )
     {
         error_value = " {connection_refused}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::connection_reset )
     {
         error_value = " {connection_reset}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::bad_descriptor )
     {
         error_value = " {bad_descriptor}";
     }
     else if( error == asio::error::fault )
     {
         error_value = " {fault}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::host_unreachable )
     {
         error_value = " {host_unreachable}";
     }
     else if( error == asio::error::in_progress )
     {
         error_value = " {in_progress}";
     }
     else if( error == asio::error::interrupted )
     {
         error_value = " {interrupted}";
     }
     else if( error == asio::error::invalid_argument )
     {
         error_value = " {invalid_argument}";
     }
     else if( error == asio::error::message_size )
     {
         error_value = " {message_size}";
     }
     else if( error == asio::error::name_too_long )
     {
         error_value = " {name_too_long}";
     }
     else if( error == asio::error::network_down )
     {
         error_value = " {network_down}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::network_reset )
     {
         error_value = " {network_reset}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::network_unreachable )
     {
         error_value = " {network_unreachable}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::no_descriptors )
     {
         error_value = " {no_descriptors}";
     }
     else if( error == asio::error::no_buffer_space )
     {
         error_value = " {no_buffer_space}";
     }
     else if( error == asio::error::no_memory )
     {
         error_value = " {no_memory}";
     }
     else if( error == asio::error::no_permission )
     {
         error_value = " {no_permission}";
     }
     else if( error == asio::error::no_protocol_option )
     {
         error_value = " {no_protocol_option}";
     }
     else if( error == asio::error::no_such_device )
     {
         error_value = " {no_such_device}";
     }
     else if( error == asio::error::not_connected )
     {
         error_value = " {not_connected}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::not_socket )
     {
         error_value = " {not_socket}";
     }
     else if( error == asio::error::operation_aborted )
     {
         error_value = " {operation_aborted}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::timed_out )
     {
         error_value = " {timed_out}";
     }
     else if( error == asio::error::try_again )
     {
         error_value = " {try_again}";
     }
     else if( error == asio::error::would_block )
     {
         error_value = " {would_block}";
     }
     else if( error == asio::error::host_not_found )
     {
         error_value = " {host_not_found}";
     }
     else if( error == asio::error::host_not_found_try_again )
     {
         error_value = " {host_not_found_try_again}";
     }
     else if( error == asio::error::no_data )
     {
         error_value = " {no_data}";
     }
     else if( error == asio::error::no_recovery )
     {
         error_value = " {no_recovery}";
     }
     else if( error == asio::error::service_not_found )
     {
         error_value = " {service_not_found}";
     }
     else if( error == asio::error::socket_type_not_supported )
     {
         error_value = " {socket_type_not_supported}";
     }
     else if( error == asio::error::already_open )
     {
         error_value = " {already_open}";
     }
     else if( error == asio::error::eof )
     {
         error_value = " {eof}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::shut_down )
     {
         error_value = " {shut_down}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }
     else if( error == asio::error::not_found )
     {
         error_value = " {not_found}";
     }
     else if( error == asio::error::fd_set_failure )
     {
         error_value = " {fd_set_failure}";
     }
        //Assume socket is in a bad state
     else
     {
         error_value = " {UNKNOWN}";
         status["connected_to_server"] =  false;
         status["ready_to_send"] = false;
         status["ready_to_receive"] = false;
     }

     log::toFile(ec.message()+error_value);
 }


 void DyrBot::register_connection()
 {
     request_send("PASS " + setting["connection_password"]);
     request_send("NICK " + setting["nickname"]);
     request_send("USER " + setting["username"] + " " + setting["mode"] + " * " + " :" + setting["realname"]);
 }


 void DyrBot::join(const std::string& channel)
 {
     //Split channel string up into individual channels in the case of comma seperated channels
     request_send("JOIN " + channel);
     boost::split(channels, channel, boost::is_any_of(","), boost::token_compress_on);
 }

 
 void DyrBot::part(const std::string& channel)
 {
     request_send("PART " + channel);
     std::set<std::string> part_channels;
     boost::split(part_channels, channel, boost::is_any_of(","), boost::token_compress_on);
    
     for(std::string a_channel : part_channels)
     {
         channels.erase(a_channel);
     }
 }

 
 void DyrBot::part_all(const irc_message_struct* message, irc_privmsg_struct* privmsg)
 {
     for(auto& channel: channels)
     { 
        request_send("PART " + channel);
     }
    
     channels.clear();
 }

 
 void DyrBot::privmsg(std::string target, std::string message)
 {
     request_send("PRIVMSG " + target + " :" + message);
 }

 
 void DyrBot::change_nick()
 {
     std::string nickname;

     for(int i(0); i < 8; ++i)
     { nickname += ((rng()%26)+65)+(32*(rng()%2)); }

     setting["nickname"] = nickname;
     request_send("NICK " + nickname);
 }


 void DyrBot::change_nick(std::string nickname)
 {
     setting["nickname"] = nickname;
     request_send("NICK " + nickname);
 }


 void DyrBot::change_nick(const irc_message_struct* irc_message, irc_privmsg_struct* irc_privmsg)
 {
     if(irc_privmsg->arguments.size() > 0)
     {
         if(irc_privmsg->arguments.at(0).find("") != irc_privmsg->arguments.at(0).end())
         change_nick(irc_privmsg->arguments.at(0).at("").at(0));
     }
 }


 void DyrBot::privmsg_handler(const irc_message_struct& message)
 {
     irc_privmsg_struct privmsg = parse::irc_privmsg(message, setting["command_ident"]);
     
     for(auto current_command : privmsg.command)
     {
         if(command.count(current_command))
         { command[current_command](this, &message, &privmsg); }
     }
 }


 void DyrBot::meta_command(const irc_message_struct* irc_message, irc_privmsg_struct* irc_privmsg)
 {
     std::string message;
     
     message += "nickname:";
     message += irc_privmsg->nickname;
     message += " username:";
     message += irc_privmsg->username;
     message += " target:";
     message += irc_privmsg->target;
     message += " ident:";
     message += irc_privmsg->ident;
     
     for(unsigned int i(0); i < irc_privmsg->command.size(); ++i)
     {
        message += " command[";
        message += std::to_string(i);
        message += "]:";
        message += irc_privmsg->command[i];
        message += "{";
        for(auto j: irc_privmsg->arguments[i])
        {
            message += "|";
            message += j.first;
            for(auto k: j.second)
            {
                message += " ";
                message += k;
            }
        }
        message += "}";
     }
        
     privmsg(irc_privmsg->target, message);
 }


 void DyrBot::request_disconnect(const irc_message_struct* message, irc_privmsg_struct* privmsg)
 {
     part_all();

     status["request_to_disconnect"] = true;
     status["ready_to_send"] = false;
     status["ready_to_receive"] = false;
 }


 void DyrBot::disconnect(const irc_message_struct* message, irc_privmsg_struct* privmsg)
 {
     boost::system::error_code ec;
    
     tcp_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    
     if(ec)
     {
         log::toFile("In DyrBot::disconnect for Bot {%}", bot_id);
         log::toFile(ec.message());
     }

     tcp_socket.close(ec);
    
     if(ec)
     {
         log::toFile("In DyrBot::disconnect for Bot {%}", bot_id);
         log::toFile(ec.message());
     }
    
     status["connected_to_server"] = false;
     status["request_to_disconnect"] = false;
     status["ready_to_send"] = false;
     status["ready_to_receive"] = false;
 }


 void DyrBot::notify_manager_ready()
 {
     bot_manager.notify_ready(bot_id);
 }


 void DyrBot::notify_manager(DyrError&& error)
 {
     bot_manager.notify_error(bot_id, std::move(error));
 }


 void DyrBot::substitute_variables(std::string& str)
 {
     std::size_t current_index = 0;
      
     while(current_index != std::string::npos)
     {
         std::size_t replace_start_index = str.find(setting["substitutor"], current_index);
         current_index = replace_start_index;
         
         std::size_t replace_end_index = str.find(setting["substitutor"], current_index+setting["substitutor"].length());
          
         if(replace_start_index != std::string::npos && replace_end_index != std::string::npos)
         {
             std::size_t var_start_index = replace_start_index+setting["substitutor"].length();
             std::size_t var_length = (replace_end_index-replace_start_index)-setting["substitutor"].length();
             std::size_t replace_length = var_length+2*setting["substitutor"].length();
          
             std::string variable = str.substr(
              var_start_index,
              var_length
             );
              
             if(setting.count(variable))
             {
                 str.replace(replace_start_index,replace_length,setting[variable]);
                 current_index += setting[variable].length();
             }
             else
             {
                 str.replace(replace_start_index,replace_length,"");
             }
         }
         else
         { break; }
     }
 }
}
