#ifndef DYRBOT_HPP
#define DYRBOT_HPP

#include <string>
#include <random>
#include <queue>
#include <map>

#include <boost/asio.hpp>
#include <boost/array.hpp>
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
    //Construct bot using config filename
    DyrBot(
        BotManager& bot_manager_shared,
        std::string config_filename,
        int uuid
    );

    //Request to connect to server
    bool request_connect_to_server();

    void request_disconnect();

    //Loop for continually making send and receive request
    void message_pump();

    private:

    //Initialize status variables
    void initialize_status();

    //Load config file
    bool load_config();

    //Load hardcoded default settings
    void load_default_settings();

    //Callback for request_connect_to_server
    void connect_handler(
        const system::error_code& error,
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
    void error_handler();

    void register_connection();
    void join(const std::string& channel);
    void part(const std::string& channel);
    void part_all();

    void change_nick();

    void disconnect();

    bool notify_manager_ready();
    void notify_manager(DyrError&& error);

    int uuid;

    bool stay_connected;
    bool config_loaded;
    bool ready_to_connect;
    bool ready_to_disconnect;
    bool connected_to_server;
    bool failed_connection;

    int pending_receives;
    int pending_sends;

    BotManager& bot_manager;

    std::queue<boost::array<char, 512> > recbuf;
    std::vector<std::string> unparsed_messages;
    std::queue<irc_message_struct> messages;

    std::vector<std::string> channels;

    ip::tcp::socket tcp_socket;

    std::map<std::string, std::string> setting;
    std::map<std::string, bool> status;
    std::queue<system::error_code> error_queue;

    std::default_random_engine rng;
    high_res_clock::time_point begin_time;
    high_res_clock::time_point end_time;
    high_res_clock::duration   time_to_connect;
    };
}

#endif /*DYRBOT_HPP*/
