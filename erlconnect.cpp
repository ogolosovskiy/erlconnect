#include <iostream>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <erl_interface.h>
#include <ei.h>

int main() {

    erl_init(NULL, 0);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
