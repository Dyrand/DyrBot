#include <exception>
#include <string>
#include <queue>

#include <boost/algorithm/string.hpp>

#include "Parser.hpp"
#include "message_struct.hpp"
#include "privmsg_struct.hpp"

namespace dyr
{
 namespace parse
 {
   //Inserts all fully parsable messages into the message_queue
   // and returns a partial message if it exist
   // or an empty string if it does not exist
   std::string raw_message(
     std::string message,
     std::vector<std::string>& message_queue
   )
   {
     bool partial = true;

     if(last_chars<2>(message) == "\r\n")
     {
       message.erase(message.end()-2, message.end());
       partial = false;
     }

     boost::algorithm::split(
       message_queue,
       message,
       boost::is_any_of("\r\n"),
       boost::token_compress_on
     );

     //Remove partial string from message_queue if necessary and return it
     std::string partial_string;

     if(partial)
     {
       partial_string = message_queue.back();
       message_queue.pop_back();
     }

     return partial_string;
   }

   irc_message_struct irc_message(std::string message)
   {
     if(!message.empty())
     {
       irc_message_struct message_struct;

       auto space_pos = message.find(' ',0);

       if(message.front() == ':')
       {
         message_struct.prefix = message.substr(0, space_pos);
         message.erase(0,space_pos+1);
       }

       space_pos = message.find(' ',0);
       message_struct.command = message.substr(0, space_pos);
       message.erase(0,space_pos+1);

       message_struct.parameters = message;

       return message_struct;
     }

     return irc_message_struct();
   }

   irc_privmsg_struct irc_privmsg(const irc_message_struct& message, const std::string& unique_delim)
   {
     irc_privmsg_struct privmsg_struct;

     auto delim = message.prefix.find("!~");
     privmsg_struct.nickname = message.prefix.substr(1,delim-1);

     auto at_symbol = message.prefix.find('@');
     privmsg_struct.username = message.prefix.substr(delim+2,(at_symbol-delim)-2);

     delim = message.parameters.find(' ');
     privmsg_struct.target = message.parameters.substr(0,delim);

     delim = message.parameters.find(':');
     auto unique_pos = message.parameters.find(unique_delim);
     //Check if command identifier is just after ':'
     if(unique_pos == delim+1)
     {
       while(unique_pos != std::string::npos)
       {
         privmsg_struct.ident = unique_delim;
         //Parse command
         auto begin_pos = message.parameters.find_first_not_of(' ',unique_pos+unique_delim.length());
         delim = message.parameters.find(' ',begin_pos+1);
         privmsg_struct.command.emplace_back(message.parameters.substr(begin_pos,delim-begin_pos));

         delim = message.parameters.find_first_not_of(' ',delim);

         //Parse after_command
         unique_pos = message.parameters.find(unique_delim,delim);
         while(unique_pos != std::string::npos && message.parameters.at(unique_pos-1) != ' ')
         { unique_pos = message.parameters.find(unique_delim,unique_pos+1); }

         if(unique_pos != std::string::npos)
         { privmsg_struct.after_command.emplace_back(message.parameters.substr(delim,unique_pos-delim)); }
         else if(delim != std::string::npos)
         { privmsg_struct.after_command.emplace_back(message.parameters.substr(delim,std::string::npos)); }
         else
         { privmsg_struct.after_command.emplace_back("");}

         //Parse command arguments
         std::string active_modifier;
         privmsg_struct.arguments.emplace_back(std::map<std::string,std::vector<std::string> >());
         while(delim != unique_pos && delim != std::string::npos)
         {
           auto end_pos = message.parameters.find(' ',delim);

           std::string temp_string(message.parameters.substr(delim,end_pos-delim));
           if(temp_string.front() == '-')
           {
             privmsg_struct.arguments.back()[temp_string] = std::vector<std::string>();
             active_modifier = temp_string;
           }
           else
           { privmsg_struct.arguments.back()[active_modifier].emplace_back(std::move(temp_string)); }


           delim = message.parameters.find_first_not_of(' ',end_pos);
         }
       }
     }

     return privmsg_struct;
   }
 }
}
