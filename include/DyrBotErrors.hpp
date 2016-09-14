#ifndef DYRBOTERRORS_HPP
#define DYRBOTERRORS_HPP

namespace dyr
{
  enum class DyrError
  {
    load_config,
    request_connect_to_server,
    connect_handler,
    send_handler,
    receive_handler
  };
}

#endif
