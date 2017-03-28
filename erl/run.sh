#/bin/bash

# . /usr/lib/erlang19/activate
. /usr/local/erl18/activate
rm *.beam
erl -compile ./bws_call_logic_test.erl
erl -name bws@192.168.88.103 -setcookie button-ws -s bws_call_logic_test



