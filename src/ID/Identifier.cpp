#include <limits>
#include <deque>


#include "Identifier.hpp"
#include "Logger.hpp"

namespace dyr
{
 /* Static member initialization */
 int id::highest_id = 0;
 int id::current_id = 0;
 std::deque<int> id::available;

 /* id::generate returns a positive integer value
    if an id is available and -1 otherwise */
 int id::generate()
 {
     if(!available.empty())
     {
         current_id = available.front();
         available.pop_front();

         log::toFile("ID{%} generated from available vector", current_id);
     }
     else if( highest_id != std::numeric_limits<int>::max())
     {
         current_id = highest_id;
         ++highest_id;

         log::toFile("ID{%} generated from max_id", current_id);
     }
     else
     {
         current_id = -1;

         log::toFile("Failed to generate a ID");
     }

     return current_id;
 }

 /* id::release makes an id available again */
 void id::release(int releasable_id)
 {
     available.push_back(releasable_id);
     log::toFile("ID{%} released", releasable_id);
 }
}
