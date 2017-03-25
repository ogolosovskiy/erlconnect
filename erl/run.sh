#/bin/bash

. /usr/lib/erlang19/activate
rm *.beam
erl -compile ./bws_call_logic.erl
erl -name bws@192.168.1.80 -setcookie button-ws -s bws_call_logic
