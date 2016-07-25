#include <limits>

#include "UniqueIdentifier.hpp"

namespace dyr
{
  //Static member variable initialization
  uuid::max_uuid = 0;


  /* uuid::generate returns a positive integer value
     if a uuid is available and -1 otherwise */
  int uuid::generate()
  {
    if(!available.empty())
    {
      current_uuid = available.front();
      available.pop_front();
    }
    else if( max_uuid != std::numeric_limits<int>())
    {
      current_uuid = max_uuid;
      ++max_uuid;
    }
    else
    {
      current_uuid = -1;
    }

    return current_uuid;
  }

  /* uuid::release makes a uuid available again */
  void uuid::release(int releasable_uuid)
  {
    available.push_back(releasable_uuid);
  }
}
