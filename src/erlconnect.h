
// std
#include <string>
#include <iostream>
#include <thread>
#include <memory>
#include <future>
#include <map>
#include <functional>

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

typedef std::map<std::string, std::string> prepare_call_response;
typedef std::future<prepare_call_response> prepare_call_future;

class erlang_client
{
public:
    erlang_client(std::string const& local_name,
                  std::string const& local_host,
                  std::string const& remote_node,
                  std::string const& cookie);
    void loop();
    prepare_call_future to_prepare_call(std::string const& to, std::string const& from);

protected:
    std::promise<prepare_call_response> _prepare_call_result;
protected:
    int _sock_tcp;
};

