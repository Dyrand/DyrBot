#ifndef IDENTIFIER_HPP
#define IDENTIFIER_HPP

#include <deque>

namespace dyr
{
    class id
    {
        public:
            static int generate();
            static void release(int releasable_id);
        private:
            static int highest_id;
            static int current_id;
            static std::deque<int> available;
    };
}

#endif /*IDENTIFIER_HPP*/
