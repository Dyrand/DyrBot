#include <limits>
#include <deque>


#include "UniqueIdentifier.hpp"
#include "Logger.hpp"

namespace dyr
{
  /* Static member initialization */
  int uuid::max_uuid = 0;
  int uuid::current_uuid = 0;
  std::deque<int> uuid::available;

  /* uuid::generate returns a positive integer value
     if a uuid is available and -1 otherwise */
  int uuid::generate()
  {
    if(!available.empty())
    {
      current_uuid = available.front();
      available.pop_front();

      #ifdef DEBUG
        log::toFile("UUID{%} generated from available vector", current_uuid);
      #endif
    }
    else if( max_uuid != std::numeric_limits<int>::max())
    {
      current_uuid = max_uuid;
      ++max_uuid;

      #ifdef DEBUG
        log::toFile("UUID{%} generated from max_uuid", current_uuid);
      #endif
    }
    else
    {
      current_uuid = -1;

      #ifdef DEBUG
        log::toFile("Failed to generate a UUID");
      #endif
    }

    return current_uuid;
  }

  /* uuid::release makes a uuid available again */
  void uuid::release(int releasable_uuid)
  {
    available.push_back(releasable_uuid);
    #ifdef DEBUG
      log::toFile("UUID{%} released", releasable_uuid);
    #endif
  }
}
