#include <string>

#include "BasicSocketInterface"

class SyncSocketInterface: public BasicSocketInterface
{
    public:
        virtual send(std::string str) = 0;
        virtual receive(std::string str) = 0;
}