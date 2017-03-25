
#include "erlconnect.h"


client::client(std::string const& erlang_node_long, std::string const& cookie)
: _sock_tcp(0)
{
    erl_init(NULL, 0);

    in_addr addr;
    addr.s_addr = inet_addr("192.168.1.80");

    if (!erl_connect_xinit((char*)"ubuntu64",
                           (char*)"node777",
                           (char*)"node777@192.168.1.80",
                           &addr,
                           (char*)(cookie.c_str()),
                           10))
        throw std::runtime_error("erl_connect_init failed" );

    const char* this_node_name = erl_thisnodename();

    std::cout << "Node:" << this_node_name << " " << cookie << std::endl;
    std::cout << "Connecting to:" << erlang_node_long << std::endl;

    if ((_sock_tcp = erl_connect(const_cast<char*>(erlang_node_long.c_str()))) < 0) {
        switch(_sock_tcp) {
            case ERL_NO_DAEMON:
                erl_err_quit("<ERROR> No epmd running!");
                break;
            case ERL_CONNECT_FAIL:
                erl_err_quit("<ERROR> Connect failed!");
                break;
            case ERL_NO_PORT:
                erl_err_quit("<ERROR> Target node is not running!");
                break;
            case ERL_TIMEOUT:
                erl_err_quit("<ERROR> Connect timed out!");
                break;
            default:
                erl_err_quit("<ERROR> Error during connect! (%d)", _sock_tcp);
                break;
        }
        throw std::runtime_error("erl_connect");
    }
    std::cout << "Success connected" << std::endl;
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

                    etrem_ptr header_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 0));

                    bool request = false;

                    if(header_term && ERL_IS_ATOM(header_term.get())) {
                            char* elem1_atom = ERL_ATOM_PTR(header_term);
                            request = std::string(elem1_atom) == "call";
                    }

                    if(request)
                    {

                        etrem_ptr from_pid_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 1));
                        etrem_ptr function_tuple_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 2));
                        etrem_ptr fun_name_term = make_eterm(ERL_TUPLE_ELEMENT(function_tuple_term, 0));
                        char *fun_name_atom = ERL_ATOM_PTR(fun_name_term);
                        etrem_ptr fun_args_term = make_eterm(ERL_TUPLE_ELEMENT(function_tuple_term, 1));

                        ETERM *resp;

                        std::cout << "fun:" << fun_name_atom << std::endl;
                        erl_print_term(stdout, fun_args_term.get());
                        std::cout << std::endl;

                        if ((resp = erl_format((char*)"{cnode, ~w}", erl_mk_atom("ok"))) != NULL) {
                            if(!erl_send(_sock_tcp, from_pid_term.get(), resp))
                                std::cout << "node " << erl_thisnodename() << ": send reply error" << std::endl;
                        } else {
                            std::cout << "node " << erl_thisnodename() << ": term format error" << std::endl;
                        }

                    } else {
                        etrem_ptr arg1_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 0));
                        etrem_ptr arg2_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 1));

                        std::cout << "incoming message:";
                        if(arg1_term)
                            erl_print_term(stdout, arg1_term.get());
                        std::cout << " ";
                        if(arg2_term)
                            erl_print_term(stdout, arg2_term.get());
                        std::cout << std::endl;
                    }
                }
                break;
            default:
                std::cout << "node " << erl_thisnodename() << ": got unexpected msg " << got << std::endl;
                break;
        }
    }
}


void client::to_prepare_call()
{
    // erl_rpc_to() returns 0 on success, otherwise a negative number.
    etrem_ptr args = make_eterm(erl_format((char*)"[~a,~a]", "Arg1", "Arg2"));
    if(0==erl_rpc_to(_sock_tcp, (char*)"bws_call_logic", (char*)"to_prepare_call", args.get()))
        std::cout << "to_prepare_call success" << std::endl;
    else
        std::cout << "to_prepare_call failed" << std::endl;
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