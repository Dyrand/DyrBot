#include <queue>
#include <memory>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <fstream>
#include <ctime>
#include <string>
#include <utility>
#include <functional>
#include <map>

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/system/error_code.hpp>

#include "DyrBot.hpp"
#include "parsers/parsers.hpp"
#include "error/error_logger.hpp"
#include "modules/module.hpp"

namespace dyr
{
  namespace asio = boost::asio;
  namespace system = boost::system;
  namespace filesystem = boost::filesystem;
  namespace placeholders = asio::placeholders;

  DyrBot::DyrBot(
      asio::io_service& io_service_,
      std::string&& config_file_path,
      std::string&& id_number):
    tcp_socket(io_service_),
    config_file("config/"+config_file_path),
    stay_connected(true),
    ready_to_process(false),
    pending_receives(0)
  {
    var["connection_password"] = "password";
    var["nickname"]            = "DyrBot";
    var["username"]            = "DyrBot";
    var["mode"]                = "0";
    var["realname"]            = "Dyramic";
    var["default_channels"]    = "#dyrbot,#botdever";
    var["id_number"]           = id_number;
    var["ident"]               = ">";
    var["custodian"]           = "";

    //Add all module commands to r_commands
    for(auto &module : module::ModuleManager::get()->module_vec)
    {
      module.init(shared_from_this());
      for(auto &command_pair : module.commands)
      {
        r_commands.at(command_pair.first) = command_pair.second;
      }
    }
  }

  void DyrBot::configure()
  {
    //Configure with default config file
    if(config_file.string() == "config/")
    { config_file /= "config.txt"; }
    //Configure with config file corresponding with id_number
    else if(config_file.string() == "config/bot")
    { config_file = filesystem::path("config/bot" + var["id_number"] + "/config.txt"); }

    if(!filesystem::is_regular_file(config_file))
    {
      if(!filesystem::is_directory((config_file.parent_path())))
      { filesystem::create_directory(config_file.parent_path()); }

      generate_config_file(config_file);
    }
    else
    {
      std::ifstream config_in(config_file.string());
      std::string line;
      std::string key;
      std::string value;

      config_in >> line;
      while(!config_in.fail())
      {
        auto pos = line.find('=');
        key = line.substr(0,pos);
        value = line.substr(pos+1);
        var[key] = value;
        config_in >> line;
      }
    }
  }

  void DyrBot::save_config()
  {
    std::ofstream config_out;

    //Assumes config_path is a directory and a filename or just a filename
    if(!filesystem::is_regular_file(config_file))
    { filesystem::create_directory(config_file.parent_path()); }

    config_out.open(config_file.string(), std::ios_base::out | std::ios_base::trunc);
    if(config_out.fail())
    {
      logError::toFile("Unable to open config file " + config_file.string());
      return;
    }

    //Output to config_file
    for(auto& pairs : var)
    { config_out << pairs.first << "=" << pairs.second << std::endl; }

    config_out.close();
  }

  void DyrBot::generate_config_file(const filesystem::path& config_path)
  {
    std::ofstream config_out;

    //Assumes config_path is a directory and a filename or just a filename
    if(!filesystem::is_regular_file(config_path))
    { filesystem::create_directory(config_path.parent_path()); }

    config_out.open(config_path.string());
    if(config_out.fail())
    {
      logError::toFile("Unable to open config file " + config_path.string());
      return;
    }

    //Output to config_file
    for(auto& pairs : var)
    { config_out << pairs.first << "=" << pairs.second << std::endl; }

    config_out.close();
  }


  void DyrBot::load_file()
  {

  }

  void DyrBot::register_connection()
  {
    send("PASS " + var["connection_password"]);
    send("NICK " + var["nickname"]);
    send("USER " + var["username"] + " " + var["mode"] + " * " + " :" + var["realname"]);
  }

  void DyrBot::change_nick(const std::string& nick)
  {
    if(var["nickname"] == nick || nick == "")
    {
      if(!test_nicks.empty())
      {
        var["nickname"] = test_nicks.front();
        test_nicks.pop();
      }
      else
      {
        var["id_number"] = std::to_string(random_num());
        var["nickname"] = var["username"] + "|" + var["id_number"];
      }
    }
    else
    { var["nickname"] = nick; }

    send("NICK " + var["nickname"]);
  }

  void DyrBot::join(const std::string& channel)
  {
    send("JOIN " + channel);
  }

  void DyrBot::privmsg(const std::string& target, const std::string& message)
  {
    send("PRIVMSG " + target + " :" + message);
  }

  void DyrBot::process()
  {
    register_connection();
    while(stay_connected)
    {
      if(pending_receives < 4)
      { receive(); }
      if(msg_queue.size() > 0)
      { message_handler(); }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  bool DyrBot::ready()
  { return ready_to_process; }

  void DyrBot::send(std::string&& text)
  {
    std::cout << "(SENDING):" << text << std::endl;
    auto binded_send_handler = boost::bind(&DyrBot::send_handler, shared_from_this(),
      placeholders::error, placeholders::iterator);

    text += "\r\n";
    std::shared_ptr<std::vector<char> > sendbuf = std::make_shared<std::vector<char> >(text.begin(),text.end());
    tcp_socket.async_send(asio::buffer(*sendbuf), binded_send_handler);
  }

  void DyrBot::send(const std::string& text)
  {
    std::cout << "(SENDING):" << text << std::endl;
    auto binded_send_handler = boost::bind(&DyrBot::send_handler, shared_from_this(),
      placeholders::error, placeholders::iterator);

    std::shared_ptr<std::vector<char> > sendbuf = std::make_shared<std::vector<char> >(text.begin(),text.end());
    sendbuf->push_back('\r');
    sendbuf->push_back('\n');
    tcp_socket.async_send(asio::buffer(*sendbuf), binded_send_handler);
  }

  void DyrBot::receive()
  {
    auto binded_receive_handler =
      boost::bind(
        &DyrBot::receive_handler,
        shared_from_this(),
        placeholders::error,
        placeholders::iterator);

    recbuf.emplace();
    tcp_socket.async_receive(asio::buffer(recbuf.back()), binded_receive_handler);
    ++pending_receives;
  }

  void DyrBot::connect(asio::ip::tcp::resolver::iterator& endpoint_iterator)
  {
    //Binded connect_handler
    auto connection_handler =
      boost::bind(
        &DyrBot::connect_handler,
        shared_from_this(),
        placeholders::error,
        placeholders::iterator);

    asio::async_connect(tcp_socket, endpoint_iterator, connection_handler);
    begin_time = std::chrono::high_resolution_clock::now();
  }

  void DyrBot::connect_handler(
    const system::error_code& ec,
    asio::ip::tcp::resolver::iterator iter)
  {
    end_time = std::chrono::high_resolution_clock::now();
    time_to_connect = end_time - begin_time;
    random_num.seed(time_to_connect.count());
    if(var["id_number"] == "-1")
    { var["id_number"] = std::to_string(random_num()); }
    test_nicks.emplace(var["nickname"] += "|" + var["id_number"]);

    if(!ec)
    {
      ready_to_process = true;
      configure();
    }
    else
    {
      logError::toFile(ec.message());
      std::this_thread::sleep_for(std::chrono::minutes(30));
    }
  }

  void DyrBot::send_handler(
    const system::error_code& ec,
    std::size_t bytes_sent)
  {
    if(ec)
    { logError::toFile(ec.message()); }
  }

  void DyrBot::receive_handler(
    const boost::system::error_code& ec,
    std::size_t bytes_received)
  {
    --pending_receives;
    if(ec)
    {
      logError::toFile(ec.message());
      std::this_thread::sleep_for(std::chrono::minutes(30));
    }
    else
    {
      std::vector<std::string> messages;

      recv_string = std::string(
        recbuf.front().begin(),
        recbuf.front().begin()+bytes_received
      );

      bool partial = true;
      if(recv_string.substr(recv_string.length()-2, 2) == "\r\n")
      {
        recv_string.erase(recv_string.length()-2, 2);
        partial = false;
      }

      boost::algorithm::split(messages, recv_string, boost::is_any_of("\r\n"), boost::token_compress_on);
      //Insert partial string to beginning of received string
      if(partial_string != "")
      {
        messages.front().insert(0,partial_string);
        partial_string.clear();
      }
      //If received string contained a partial string append to partial_string
      if(partial)
      {
        partial_string += messages.back();
        messages.pop_back();
      }

      recbuf.pop();

      for(std::string& text : messages)
      {
        std::cout << text << std::endl;
        msg_queue.push(parseMessage(text));
      }
    }
  }

  void DyrBot::message_handler()
  {
    while(!msg_queue.empty())
    {
      message = msg_queue.front();

      if(message.command == "PING")
      { send("PONG " + message.parameters); }
      else if(message.command == "PRIVMSG")
      { privmsg_handler(message); }
      else if(message.command == "001") //RPL_WELCOME
      { join(var["default_channels"]); }
      else if(message.command == "433") //ERR_NICKNAMEINUSE
      { change_nick(""); }
      else if(message.command == "436") //ERR_NICKCOLLISION
      { change_nick(""); }

      msg_queue.pop();
    }
  }

  void DyrBot::privmsg_handler(const Message_Struct& message)
  {
    Privmsg_Struct msg(parsePrivmsg(message,var["ident"]));
    for(int i(0); i < msg.command.size(); ++i)
    {
      if(!r_commands.at(msg.command.at(i)).get().restriction(msg))
      { r_commands.at(msg.command.at(i)).get().command(msg,i); }
      else if(!msg.command.at(i).empty())
      {
        if(msg.command.at(i).front() == '!')
        {
            if(msg.nickname == var["custodian"])
            {
              msg.command.at(i).erase(0,1);
              if(msg.command.at(i) == "save_config")
              {
                save_config();
                privmsg(msg.target,"Configuration file saved!");
              }
              else if(msg.command.at(i) == "set")
              {
                if(msg.arguments.at(i)[""].size() >= 2)
                {
                  if(msg.arguments.at(i)[""].at(0) == "nickname")
                  { change_nick(msg.arguments.at(i)[""].at(1)); }
                  else
                  {
                    var[msg.arguments.at(i)[""].at(0)] = msg.arguments.at(i)[""].at(1);
                    privmsg(msg.target,msg.arguments.at(i)[""].at(0)+" set to "+msg.arguments.at(i)[""].at(1));
                  }
                }
                else
                { privmsg(msg.target,"Not enough parameters @"+msg.nickname); }
              }
              /*
              else if(msg.command.at(i) == "see")
              {
                if(msg.arguments.at(i)[""].size() >= 1)
                {
                  std::string temp;
                  for(auto& str: msg.arguments.at(i)[""])
                  { temp += str + "=\"" + var[str] + "\" "; }
                  privmsg(msg.target,temp);
                }
              }*/
              else if(msg.command.at(i) == "raw")
              { send(msg.after_command.at(i)); }
            }
            else
            { privmsg(msg.target,"Must be a custodian to use '!' commands"); }
        }
      }
    }
  }
}
