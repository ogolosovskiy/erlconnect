-module(testserver).
-export([call_cnode/1]).


call_cnode(Msg) ->
    {any, 'node777@192.168.1.80'} ! {call, self(), Msg},
    receive
	    {cnode, Result} ->
	        Result
    end.
