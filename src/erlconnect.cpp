
#include <erl_interface.h>
#include "erlconnect.h"

prepare_call_response map_from_elist(ETERM* etrem)
{
    prepare_call_response result;
    ETERM* epl = etrem;
    if(epl && ERL_IS_LIST(epl))
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
    , _ref(0)
{
    erl_init(NULL, 0);

    in_addr addr;
    addr.s_addr = inet_addr("192.168.88.103");
    _node_name = local_name + "@" + local_host;
    if (!erl_connect_xinit((char*)"localhost",
                           (char*)local_name.c_str(),
                           (char*)_node_name.c_str(),
                           &addr,
                           (char*)(cookie.c_str()),
                           _creation))
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
                  эти мудаки захардкодили таг, придется писать "закат солнца вручную"
                  /usr/lib/erlang18/lib/erl_interface-3.8.2/src/connect/ei_connect.c
                  ei_send_reg_encoded(fd, self, "rex", x.buff, x.index);
                  или позже переписать С++ RPC самому
                */
                if (emsg.type == ERL_REG_SEND || emsg.type ==  ERL_SEND) {

                    erl_print_term(stdout, emsg.msg);
                    std::cout << std::endl;

                    etrem_ptr header_term = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 0));
                    etrem_ptr header_term2 = make_eterm(ERL_TUPLE_ELEMENT(emsg.msg, 1));

                    /* protocol expected
                    RPC Request
                     {'RPC_Request', RPC_Id_Ref, [Args]}
                    RPC Response
                     {'RPC_Response', RPC_Id_Ref, [Args]}
                    Simple message
                     {'Message', from_pid(), [Args]}
                    */

                    message_type type = Unknown;
                    int ref = 0;

                    if(header_term && ERL_IS_ATOM(header_term.get())) {
                        std::string header_fake_ref(ERL_ATOM_PTR(header_term.get()));
                        if(header_fake_ref=="rex")
                        {
                            if(header_term2 && ERL_IS_TUPLE(header_term2.get())) {
                                etrem_ptr header_elem1 = make_eterm ERL_TUPLE_ELEMENT(header_term2.get(), 0);
                                if(header_elem1 && ERL_IS_ATOM(header_elem1.get())) {
                                    std::string header_name(ERL_ATOM_PTR(header_elem1.get()));
                                    if(header_name=="RPC_Request")
                                        type = RPC_Request;
                                    if(header_name=="RPC_Response")
                                        type = RPC_Response;
                                }
                                etrem_ptr header_elem2 = make_eterm ERL_TUPLE_ELEMENT(header_term2.get(), 1);
                                if(header_elem2 && ERL_IS_REF(header_elem2.get())) {
                                    ref = ERL_REF_NUMBER(header_elem2.get());
                                }
                            }


                        }
                    }

                     switch(type)
                    {
                        case Message: {
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
                                if (!erl_send(_sock_tcp, from_pid_term.get(), resp))
                                    std::cout << "node " << erl_thisnodename() << ": send reply error" << std::endl;
                            }
                        }
                        break;
                        case RPC_Response: {
                            std::cout << "RPC_Response ref: " << ref << std::endl;
                            etrem_ptr arg2_term = make_eterm(ERL_TUPLE_ELEMENT(header_term2, 2));
                            prepare_call_response mp = map_from_elist(arg2_term.get());

                            prepare_call_promise promise;
                            {
                                std::lock_guard<std::mutex> guard(_prepare_call_guard);
                                auto prm = _prepare_call_results.find(ref);
                                if (prm != _prepare_call_results.end()) {
                                    promise = prm->second;
                                    _prepare_call_results.erase(prm);
                                }
                            }
                            if(promise)
                                promise->set_value(mp);
                        }
                        break;
                        case RPC_Request:
                            std::cout << "RPC_Request:" << ref << std::endl;
                            break;
                        default:
                            std::cout << "node " << erl_thisnodename() << ": term format error" << std::endl;
                    }
                }
                break;
            default:
                std::cout << "node " << erl_thisnodename() << ": got unexpected msg " << got << std::endl;
                break;
        }
    }
}


etrem_ptr erlang_client::make_ref()
{
    _ref++;
    if(_ref>=262143)
        _ref = 0;
    // old Erlang reference, with only 18 bits
    return make_eterm(erl_mk_ref(_node_name.c_str(), _ref, _creation));
}

int erlang_client::get_ref(etrem_ptr const& term) const
{
    return ERL_REF_NUMBER(term.get()); //term->uval.refval.n[0];
}

prepare_call_future erlang_client::to_prepare_call(std::string const& to, std::string const& from)
{
    etrem_ptr to_bs_term = make_eterm(erl_mk_binary("to", sizeof("to")-1));
    etrem_ptr from_bs_term = make_eterm(erl_mk_binary("from", sizeof("from")-1));
    etrem_ptr toSEQ_bs_term = make_eterm(erl_mk_binary(to.c_str(), to.size()-1));
    etrem_ptr fromSEQ_bs_term = make_eterm(erl_mk_binary(from.c_str(), from.size()-1));
    etrem_ptr ref = make_ref();

    prepare_call_promise promise(new std::promise<prepare_call_response>());
    prepare_call_future future = promise->get_future();
    // TO DO not thread safe
    int n = get_ref(ref);
    {
        std::lock_guard<std::mutex> guard(_prepare_call_guard);
        _prepare_call_results[n] = promise;
    }

    // {'RPC_Request', RPC_Id}
    etrem_ptr arg = make_eterm(erl_format((char*)"[{~a, ~w, [{~w, ~w},{~w, ~w}]}]"
            , "RPC_Request", ref.get()
            , to_bs_term.get(), toSEQ_bs_term.get()
            , from_bs_term.get(), fromSEQ_bs_term.get()));
    if(0==erl_rpc_to(_sock_tcp, (char*)"bws_call_logic_test", (char*)"to_prepare_call", arg.get())) {
        return future;
    }
    else {
        std::cout << "to_prepare_call failed" << std::endl;
        throw std::runtime_error("to_prepare_call failed");
    }
}

