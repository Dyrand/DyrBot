#include <iostream>

#include "BotManager.hpp"

int main(int argc, char *argv[])
{
  dyr::BotManager manager;

  int id = manager.createBot();
  manager.deleteBot(id);
}
