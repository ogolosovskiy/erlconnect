
#include "erlconnect.h"



res_map map_from_elist(ETERM* etrem)
{
//    erl_print_term(stdout, etrem);
    res_map result;
    ETERM* epl = etrem;
    if(ERL_IS_LIST(epl))
    {
//        std::cout << std::endl;
        //      putc('[', stdout);
        while (ERL_IS_CONS(epl)) {
            ETERM* ept = epl->uval.lval.head;
            if(ERL_IS_TUPLE(ept)) {
                //            putc('{', stdout);
                unsigned char *key = 0;
                unsigned char *val = 0;

                for (int i = 0; i < ERL_TUPLE_SIZE(ept); i++) {
                    ETERM* epb1 = ERL_TUPLE_ELEMENT(ept, 0);
                    if( ERL_IS_BINARY(epb1)) {
//                        int sz = (ERL_BIN_SIZE(epb1) > 20) ? 20 : ERL_BIN_SIZE(epb1);
                        unsigned char *ptr = ERL_BIN_PTR(epb1);
                        key = ptr;
                        //                  for (i = 0; i < sz; i++) {
                        //                    putc(ptr[i], stdout);
                        //              }
                    }
                    if (i != ERL_TUPLE_SIZE(ept) - 1) {
                        putc(',', stdout);
                    }
                    ETERM* epb2 = ERL_TUPLE_ELEMENT(ept, 1);
                    if( ERL_IS_BINARY(epb2)) {
                        //          int sz = (ERL_BIN_SIZE(epb2) > 20) ? 20 : ERL_BIN_SIZE(epb2);
                        unsigned char *ptr = ERL_BIN_PTR(epb2);
                        val = ptr;
                        //            for (i = 0; i < sz; i++) {
                        //              putc(ptr[i], stdout);
                        //        }
                    }
                    if(key && val)
                        result[((char const*)key)] = std::string((char const*)val);
                }
                putc('}', stdout);
            }
            epl = epl->uval.lval.tail;
            if (ERL_IS_CONS(epl)) {
                putc(',', stdout);
            }
        }
        if (!ERL_IS_EMPTY_LIST(epl)) {
            putc('|', stdout);
            // ch_written += erl_print_term(fp, ep);
        }
//        putc(']', stdout);
    }

    return result;
}

client::client(const client& copy)
: _sock_tcp(copy._sock_tcp)
{

}

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
//                        etrem_ptr arg1_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 0));
                        etrem_ptr arg2_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 1));
                        std::cout << "incoming message:";
                        _prepare_call_result.set_value(map_from_elist(arg2_term.get()));
                    }
                }
                break;
            default:
                std::cout << "node " << erl_thisnodename() << ": got unexpected msg " << got << std::endl;
                break;
        }
    }
}

void client::to_prepare_call(std::string const& to, std::string const& from)
{
    etrem_ptr to_bs_term = make_eterm(erl_mk_binary("to", sizeof("to")-1));
    etrem_ptr from_bs_term = make_eterm(erl_mk_binary("from", sizeof("from")-1));
    etrem_ptr toSEQ_bs_term = make_eterm(erl_mk_binary(to.c_str(), to.size()-1));
    etrem_ptr fromSEQ_bs_term = make_eterm(erl_mk_binary(from.c_str(), from.size()-1));

    std::future<res_map> ready = _prepare_call_result.get_future();
    res_map ff;

    etrem_ptr arg = make_eterm(erl_format((char*)"[[{~w, ~w},{~w, ~w}]]", to_bs_term.get(), toSEQ_bs_term.get(), from_bs_term.get(), fromSEQ_bs_term.get()));
    if(0==erl_rpc_to(_sock_tcp, (char*)"bws_call_logic_test", (char*)"to_prepare_call", arg.get())) {
        std::cout << "to_prepare_call success" << std::endl;
        ready.wait();
        ff = ready.get();
    }
    else
        std::cout << "to_prepare_call failed" << std::endl;

    ff.clear();
}

void client::ls()
{
    etrem_ptr arg = make_eterm(erl_format((char*)"[~a]", "."));
    if(0==erl_rpc_to(_sock_tcp, (char*)"c", (char*)"ls", arg.get()))
        std::cout << "ls success" << std::endl;
    else
        std::cout << "ls failed" << std::endl;
}