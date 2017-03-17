#include <iostream>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <erl_interface.h>
#include <ei.h>

#include "erlconnect.h"


client::client(std::string const& erlang_node_long, std::string const& cookie)
: sock_tcp(0)
{
    std::cout << "client init: " << erlang_node_long << " " <<  cookie << std::endl;
    erl_init(NULL, 0);

    in_addr addr;
    addr.s_addr = inet_addr("134.138.177.89");

    if (!erl_connect_xinit("ubuntu64",
                           "node777",
                           "node777@192.168.1.80",
                           &addr,
                           const_cast<char*>(cookie.c_str()),
                           0))
        throw std::runtime_error("erl_connect_init failed" );


    const char* this_node_name = erl_thisnodename();
    short this_creation = erl_thiscreation();
    std::cout << "Node  " << this_node_name << " " << this_creation << std::endl;

    std::cout << "Connecting to " << erlang_node_long << std::endl;

    //hostent* host = erl_gethostbyname("ubuntu64");

    if ((sock_tcp = erl_connect(const_cast<char*>(erlang_node_long.c_str()))) < 0) {
        // std::cerr << "cant connect " << erl_errno << std::endl;
        throw std::runtime_error("erl_connect");
    }
    std::cout << "Connected to " << erlang_node_long << std::endl;
}

size_t const BUFSIZE = 1000;

void client::loop()
{

    unsigned char buf[BUFSIZE];
    ErlMessage emsg;
    int got(0);

    std::cout << "loop run" << std::endl;

    while (sock_tcp) {

        got = erl_receive_msg(sock_tcp, buf, BUFSIZE, &emsg);
        if (got == ERL_TICK) {
            /* ignore */
        } else if (got == ERL_ERROR) {
            sock_tcp = 0; // TO DO close
        } else {

            if (emsg.type == ERL_REG_SEND) {
            }
        }
    }
}
