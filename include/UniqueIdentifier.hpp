#ifndef UNIQUE_IDENTIFIER_HPP
#define UNIQUE_IDENTIFIER_HPP

#include <deque>

namespace dyr
{
  class uuid
  {
    public:
      static int generate();
      static void release(int releasable_uuid);
    private:
      static int max_uuid;
      static int current_uuid;
      static std::deque<int> available;
  };
}

#endif /*UNIQUE_IDENTIFIER_HPP*/
