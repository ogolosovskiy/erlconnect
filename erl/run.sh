#/bin/bash

rm *.beam
erl -compile ./bws_call_logic_test.erl
erl -name bws@192.168.1.80 -setcookie button-ws -s bws_call_logic_test



