#include <stdio.h>
#include "erlconnect.h"
#include <thread>
#include <iostream>

int main() {

    //  erl -name node1@192.168.1.80 -setcookie secretcookie
    client cli("node1@192.168.1.80", "secretcookie");

   std::thread t1(&client::loop, cli);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << " run test " << std::endl;
    cli.test();
    std::cout << " join thread " << std::endl;
    t1.join();

    return 0;
}
