#include <stdio.h>
#include "erlconnect.h"
#include <thread>
#include <iostream>

int main() {

    //  erl -name node1@192.168.1.80 -setcookie secretcookie
    client cli("bws@192.168.88.103", "button-ws");
    cli.test = 5;
    std::function<void(void)> f = std::bind(&client::loop, &cli);
    std::thread t1(f);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    res_map res = cli.to_prepare_call("BEE_RU_to", "BEE_RU_from");

    for(auto elem : res)
        std::cout << elem.first << " " << elem.second << std::endl;

    t1.join();

    return 0;
}
