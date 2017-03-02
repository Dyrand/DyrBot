#include <string>

class BasicSocketInterface
{
    public:
        virtual connect(std::string domain_name, int port) = 0;
        virtual disconnect();
        virtual ~BasicSocketInterface();
        
}