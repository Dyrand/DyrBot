#ifndef DYR_PARSER_HPP
#define DYR_PARSER_HPP

#include <string>
#include <vector>
#include <queue>

#include <boost/program_options.hpp>

#include "privmsg_struct.hpp"
#include "message_struct.hpp"

namespace dyr
{
namespace parse
{
    namespace po = boost::program_options;
    
    //Returns the last <count> number of characters in str as a string
    template <int count>
     std::string last_chars(const std::string& str)
     { return str.substr(str.length()-count, count); }

    //Inserts all fully parsable messages into the message_queue
    // and returns a partial message if it exist
    // or an empty string if it does not exist
    std::string raw_message(
     std::string message,
     std::vector<std::string>& message_queue
    );

    irc_message_struct irc_message(std::string message);

    irc_privmsg_struct irc_privmsg(
     const irc_message_struct& message,
     const std::string& unique_delim
    );
   
    
    template<typename charT>
      po::variables_map command_line(
       int argc,
       const charT *const argv,
       const po::options_description& desc
      )
      {
          po::variables_map vm;
          po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), vm);
          po::notify(vm);
          
          return vm;
      }
 }
}

#endif /*DYR_PARSER_HPP*/
