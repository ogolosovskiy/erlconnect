
#include <string>

class client
{
public:
    client(std::string const& erlang_node_long, std::string const& cookie);
    void loop();
    std::string node;
    int sock_tcp;
};
