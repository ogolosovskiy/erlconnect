
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
typedef std::shared_ptr<std::promise<prepare_call_response> > prepare_call_promise;
typedef std::map<int, prepare_call_promise> prepare_call_map;

class erlang_client
{
public:
    erlang_client(std::string const& local_name,
                  std::string const& local_host,
                  std::string const& remote_node,
                  std::string const& cookie);
    void loop();
    prepare_call_future to_prepare_call(std::string const& to, std::string const& from);

    etrem_ptr make_ref();
    int get_ref(etrem_ptr const& term) const;

    enum message_type
    {
        Unknown = 0,
        RPC_Request = 1,
        RPC_Response = 2,
        Message = 3
    };

protected:
    std::mutex       _prepare_call_guard;
    prepare_call_map _prepare_call_results;
protected:
    int _sock_tcp;
    static int const _creation = 17;
    int _ref;
    std::string _node_name;
};

