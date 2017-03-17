#include <iostream>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <erl_interface.h>
#include <ei.h>

#include "erlconnect.h"

void test() {
    erl_init(NULL, 0);
    std::cout << "Hello, World!" << std::endl;
}
