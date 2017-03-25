-module(bws_call_logic).
-export([start/0]).
-export([to_prepare_call/2]).

start() ->
  io:format("Escucha me!!!~n").

to_prepare_call(Msg1, Msg2) ->
  io:format("to_prepare_call ~w ~w~n", [Msg1, Msg2]),
  [Msg1, Msg2].
%%  #{key => val}.
