
// std
#include <string>
#include <iostream>
#include <thread>
#include <memory>

//stdlib
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// unix
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// erlang
#include <erl_interface.h>
#include <ei.h>

typedef std::function<void (ETERM*)> eterm_deleter;
typedef std::unique_ptr<ETERM, eterm_deleter> etrem_ptr;
etrem_ptr make_eterm(ETERM* new_one)
{
    eterm_deleter deleter(erl_free_term);
    return std::unique_ptr<ETERM, eterm_deleter>(new_one, deleter);
}

class client
{
public:
    client(std::string const& erlang_node_long, std::string const& cookie);
    void loop();
    void to_prepare_call();
protected:
    int _sock_tcp;
};
