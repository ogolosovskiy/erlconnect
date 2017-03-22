
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <erl_interface.h>
#include <ei.h>

#include "erlconnect.h"


client::client(std::string const& erlang_node_long, std::string const& cookie)
: _sock_tcp(0)
{
    std::cout << "client init: " << erlang_node_long << " " <<  cookie << std::endl;
    erl_init(NULL, 0);

    in_addr addr;
    addr.s_addr = inet_addr("134.138.177.89");

    if (!erl_connect_xinit((char*)"ubuntu64",
                           (char*)"node777",
                           (char*)"node777@192.168.1.80",
                           &addr,
                           (char*)(cookie.c_str()),
                           10))
        throw std::runtime_error("erl_connect_init failed" );

    const char* this_node_name = erl_thisnodename();
    short this_creation = erl_thiscreation();
    std::cout << "Node  " << this_node_name << " " << this_creation << std::endl;

    std::cout << "Connecting to " << erlang_node_long << std::endl;

    if ((_sock_tcp = erl_connect(const_cast<char*>(erlang_node_long.c_str()))) < 0) {
        // std::cerr << "cant connect " << erl_errno << std::endl;
        throw std::runtime_error("erl_connect");
        // To do
        /* (failure)
            case ERL_NO_DAEMON:
                erl_err_quit("<ERROR> No epmd running!");
                break;
            case ERL_CONNECT_FAIL:
                erl_err_quit("<ERROR> Connect failed!");
                break;
            case ERL_NO_PORT:
                erl_err_quit("<ERROR> Node is not running!");
                break;
            case ERL_TIMEOUT:
                erl_err_quit("<ERROR> Connect timed out!");
                break;
            default:
                erl_err_quit("<ERROR> Error during connect! (%d)",fd);
                break;
           */
    }
    std::cout << "Connected to " << erlang_node_long << std::endl;
}

ETERM *foo();
ETERM *bar();
ETERM *baz();
ETERM *quax();

const static struct _fnames {
    const char *name;
    ETERM *(*func)(void);
} fns[] = {
        { "foo", foo },
        { "bar", bar },
        { "baz", baz },
        { "quax", quax }
};

short int get_fn_idx(char *pattern) {
    size_t i = 0;
    while(i < (sizeof(fns) / sizeof(fns[0]))) {
        if (strcmp(fns[i].name, pattern) == 0)
            return i;
        i++;
    }
    return -1;
}

size_t const BUFSIZE = 1000;
void client::loop()
{

    unsigned char buf[BUFSIZE];
    ErlMessage emsg;
    int got(0);

    std::cout << "loop run" << std::endl;

    while (_sock_tcp) {

        got = erl_receive_msg(_sock_tcp, buf, BUFSIZE, &emsg);

        switch (got) {
            case ERL_TICK:
                std::cout << "keep alive packet received" << std::endl;
                break;
            case ERL_ERROR:
                std::cout << "node " << erl_thisnodename() << ": erl_receive_msg error or node down" << std::endl;
                erl_close_connection(_sock_tcp);
                _sock_tcp = 0;
                break;
            case ERL_MSG:
                /*
                  Incoming request term: {call, from_pid(), {fun_name_atom, arguments_list}}}
                  Reply term: {from_pid(), any}
                */
                if (emsg.type == ERL_REG_SEND || emsg.type ==  ERL_SEND) {

                    ETERM* header_term = ERL_TUPLE_ELEMENT(emsg.msg, 0);

                    bool request = false;

                    if(header_term && ERL_IS_ATOM(header_term)) {
                            char* elem1_atom = ERL_ATOM_PTR(header_term);
                            request = std::string(elem1_atom) == "call";
                    }

                    if(request)
                    {
                        ETERM* from_pid_term = ERL_TUPLE_ELEMENT(emsg.msg, 1);
                        ETERM* function_tuple_term = ERL_TUPLE_ELEMENT(emsg.msg, 2);
                        ETERM* fun_name_term = ERL_TUPLE_ELEMENT(function_tuple_term, 0);
                        char *fun_name_atom = ERL_ATOM_PTR(fun_name_term);
                        ETERM *fun_args_term = ERL_TUPLE_ELEMENT(function_tuple_term, 1);

                        ETERM *resp;

                        std::cout << "fun:" << fun_name_atom << std::endl;
                        erl_print_term(stdout, fun_args_term);
                        std::cout << std::endl;

                        if ((resp = erl_format("{cnode, ~w}", erl_mk_atom("ok"))) != NULL) {
                            if(!erl_send(_sock_tcp, from_pid_term, resp))
                                std::cout << "node " << erl_thisnodename() << ": send reply error" << std::endl;
                        } else {
                            std::cout << "node " << erl_thisnodename() << ": term format erro" << std::endl;
                        }

                        erl_free_term(fun_name_term);
                        erl_free_term(fun_args_term);
                        erl_free_term(resp);
                        erl_free_term(from_pid_term);
                        erl_free_term(function_tuple_term);

                    } else {

                        ETERM* arg1_term = ERL_TUPLE_ELEMENT(emsg.msg, 0);
                        ETERM* arg2_term = ERL_TUPLE_ELEMENT(emsg.msg, 1);

                        std::cout << "incoming message:";
                        if(arg1_term)
                            erl_print_term(stdout, arg1_term);
                        std::cout << " ";
                        if(arg2_term)
                            erl_print_term(stdout, arg2_term);
                        std::cout << std::endl;

                        erl_free_term(arg1_term);
                        erl_free_term(arg2_term);
                    }

                    erl_free_term(emsg.from);
                    erl_free_term(emsg.msg);
                    erl_free_term(header_term);

/*
                    ETERM *tuplep, *fnp, *argp, *resp, *f_resp;

                    tuplep = erl_element(2, emsg.msg);


                    char *atom1 = 0;
                    atom1 = ERL_ATOM_PTR(tuplep);

                    fnp = erl_element(1, tuplep);
                    argp = erl_element(2, tuplep);

                    char *atom = 0;
                    char const * call = 0;

                    atom = ERL_ATOM_PTR(fnp);

                    short int idx;
                    if((idx = get_fn_idx(atom)) >= 0) {
                        call = fns[idx].name;
                        f_resp = fns[idx].func();
                    }
                    else {
                        call = "error";
                        f_resp = erl_mk_atom("unknown_function");
                    }

                    std::cout << "node " << erl_thisnodename() << ": " << call << std::endl;

                    if ((resp = erl_format((char*)"{cnode, {reply, {~w, ~w}}}", erl_mk_atom(call), f_resp)) != NULL) {
                        if(!erl_send(_sock_tcp, fromp, resp))
                            std::cout << "node " << erl_thisnodename() << ": send reply error" << std::endl;
                    } else {
                        std::cout << "node " << erl_thisnodename() << ": term format erro" << std::endl;
                    }

                    erl_free_term(emsg.from);
                    erl_free_term(emsg.msg);
                    erl_free_term(fromp);
                    erl_free_term(tuplep);
                    erl_free_term(fnp);
                    erl_free_term(argp);
                    erl_free_term(resp);
                    erl_free_term(f_resp);
                    */
                }
                break;
            default:
                std::cout << "node " << erl_thisnodename() << ": got unexpected msg " << got << std::endl;
                break;
        }
    }
}


void client::test()
{
    ETERM *n;
    static erlang_pid pid;

    std::cout << "node " << erl_thisnodename() << ": erl_rpc" << std::endl;

    n = erl_format((char*)"[~a]", ".");
    int reply = erl_rpc_to(_sock_tcp, "c", "ls", n);
    erl_free_term(n);

    std::cout << "node " << erl_thisnodename() << ": done " << reply << std::endl;

}

/* Callbacks */

ETERM *foo() {
    return erl_mk_int(100);
}

ETERM *bar() {
    return erl_mk_atom("bar_function_returned_atom");
}

ETERM *baz() {
    return erl_mk_float(3.141592);
}

ETERM *quax() {
    return erl_mk_estring("test string", strlen("test string"));
}