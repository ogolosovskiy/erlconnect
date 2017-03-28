-module(bws_call_logic_test).
-export([start/0]).
-export([to_prepare_call/1]).

start() ->
  io:format("Escucha me!!!~n").

to_prepare_call(RequestList) ->
  try
    io:format("to_prepare_call ~p ~n", [RequestList]),
    RespMap = #{<<"SSRC_A">> => <<"29552370">>,
                                         <<"SSRC_B">> => <<"29552371">>,
                                         <<"SSTP_A">> => <<"OTQ4NTVmZDE4Y2E1NGVlNjYwMjgzZDYxMzhhMWM3">>,
                                         <<"SSTP_B">> => <<"OWQwNjllZTA1OTlhZGQ2NGViZTA1NDM4M2RlYWU2">>,
                                         <<"call_id">> => <<"25f1fd16">>,
                                         <<"server_token_from">> => <<"HCqh+WqjL9OnF6IzZwsWqMq8ZfkknPAYi/g4lJqQCKGqtPi4GNm6Ma7FS2X9L7u8">>,
                                         <<"server_token_to">> => <<"+0Pc3Qup62X/x6BNU1WnjT8PPjhuH7DndWNh2vS2Ld4=">>,
                                         <<"to_devices">> => [<<"fake_device_todo">>],
                                         <<"turn_IP">> => <<"turn:52.91.165.63:3478">>},
    io:format("to_prepare_call result: ~p ~n", [RespMap]),
    maps:to_list(RespMap)
  catch
    ClassType:Reason ->
      [{_Module, _Fun, _Arity, [{file, File}, {line, Line}]} | _ ] = Stack = erlang:get_stacktrace(),
      io:format("~p: ~p~nlocation: ~p~nstack:~n~p~n", [ClassType, Reason, lists:flatten(io_lib:format("~s ~p", [File, Line])), Stack])
  end.
