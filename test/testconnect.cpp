#include <stdio.h>
#include "erlconnect.h"



int main() {

    //  erl -name node1@192.168.1.80 -setcookie secretcookie
    client cli("node1@192.168.1.80", "secretcookie");

    cli.loop();

    return 0;
}
