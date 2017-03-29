
#include "erlconnect.h"



prepare_call_response map_from_elist(ETERM* etrem)
{
//    erl_print_term(stdout, etrem);
    prepare_call_response result;
    ETERM* epl = etrem;
    if(ERL_IS_LIST(epl))
    {
        while (ERL_IS_CONS(epl)) {
            ETERM* ept = epl->uval.lval.head;
            if(ERL_IS_TUPLE(ept)) {
                unsigned char *key = 0;
                unsigned char *val = 0;

                for (int i = 0; i < ERL_TUPLE_SIZE(ept); i++) {
                    ETERM* epb1 = ERL_TUPLE_ELEMENT(ept, 0);
                    if( ERL_IS_BINARY(epb1)) {
                        unsigned char *ptr = ERL_BIN_PTR(epb1);
                        key = ptr;
                    }
                    ETERM* epb2 = ERL_TUPLE_ELEMENT(ept, 1);
                    if( ERL_IS_BINARY(epb2)) {
                        unsigned char *ptr = ERL_BIN_PTR(epb2);
                        val = ptr;
                    }
                    if(key && val)
                        result[((char const*)key)] = std::string((char const*)val);
                }
            }
            epl = epl->uval.lval.tail;
        }
    }

    return result;
}

erlang_client::erlang_client(
        std::string const& local_name,
        std::string const& local_host,
        std::string const& remote_node,
        std::string const& cookie)
    : _sock_tcp(0)
{
    erl_init(NULL, 0);

    in_addr addr;
    addr.s_addr = inet_addr("192.168.88.103");

    std::string local_node = local_name + "@" + local_host;

    if (!erl_connect_xinit((char*)"localhost",
                           (char*)local_name.c_str(),
                           (char*)local_node.c_str(),
                           &addr,
                           (char*)(cookie.c_str()),
                           17))
        throw std::runtime_error("erl_connect_init failed" );

    const char* this_node_name = erl_thisnodename();

    std::cout << "Node:" << this_node_name << " " << cookie << std::endl;
    std::cout << "Connecting to:" << remote_node << std::endl;

    if ((_sock_tcp = erl_connect(const_cast<char*>(remote_node.c_str()))) < 0) {
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
    std::cout << remote_node << " connected" << std::endl;
}

size_t const BUFSIZE = 1000;
void erlang_client::loop()
{

    unsigned char buf[BUFSIZE];
    ErlMessage emsg;
    int got(0);

    std::cout << "loop running..." << std::endl;

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
//                        etrem_ptr arg1_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 0));
                        etrem_ptr arg2_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 1));
                        prepare_call_response mp = map_from_elist(arg2_term.get());
                        _prepare_call_result.set_value(mp);
                    }
                }
                break;
            default:
                std::cout << "node " << erl_thisnodename() << ": got unexpected msg " << got << std::endl;
                break;
        }
    }
}

prepare_call_future erlang_client::to_prepare_call(std::string const& to, std::string const& from)
{
    etrem_ptr to_bs_term = make_eterm(erl_mk_binary("to", sizeof("to")-1));
    etrem_ptr from_bs_term = make_eterm(erl_mk_binary("from", sizeof("from")-1));
    etrem_ptr toSEQ_bs_term = make_eterm(erl_mk_binary(to.c_str(), to.size()-1));
    etrem_ptr fromSEQ_bs_term = make_eterm(erl_mk_binary(from.c_str(), from.size()-1));

    std::future<prepare_call_response> future = _prepare_call_result.get_future();

    etrem_ptr arg = make_eterm(erl_format((char*)"[[{~w, ~w},{~w, ~w}]]", to_bs_term.get(), toSEQ_bs_term.get(), from_bs_term.get(), fromSEQ_bs_term.get()));
    if(0==erl_rpc_to(_sock_tcp, (char*)"bws_call_logic_test", (char*)"to_prepare_call", arg.get())) {
        return future;
    }
    else {
        std::cout << "to_prepare_call failed" << std::endl;
        throw std::runtime_error("to_prepare_call failed");
    }
}

