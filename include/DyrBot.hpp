#ifndef DYRBOT_HPP
#define DYRBOT_HPP

#include <memory>

namespace dyr
{
  class BotManager;

  class DyrBot
  {
    public:
      DyrBot() = default;
      ~DyrBot() = default;
      DyrBot(const DyrBot&) = default; //Copy Constructor
      DyrBot(DyrBot&& other) noexcept = default; //Move Constructor
      DyrBot& operator=(const DyrBot& other) = default; //Copy Assignment
      DyrBot& operator=(DyrBot&& other) noexcept = default; //Move Assignment

    private:
      std::weak_ptr<BotManager> manager;
  };
}

#endif /*DYRBOT_HPP*/
