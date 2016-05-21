#include <iostream>
#include <chrono>
#include <thread>

#include "ConnectionManager.hpp"

int main(int argc, char *argv[])
{
  dyr::ConnectionManager manager;
  manager.connect("irc.freenode.com","8001");
  manager.connect("irc.freenode.com","8001","bot","42");
  manager.process();
  return 0;
}
