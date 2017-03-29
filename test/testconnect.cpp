#include <stdio.h>
#include "erlconnect.h"
#include <thread>
#include <iostream>

int main() {

    erlang_client cli("node777", "192.1.68.1.80", "bws@192.168.1.80", "button-ws");
    std::thread t1(std::bind(&erlang_client::loop, &cli)); // macosx specific, please dont use std:ref(&cli)

    for(int i = 0;i<3;i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        prepare_call_future fut = cli.to_prepare_call("BEE_RU_to", "BEE_RU_from");
        fut.wait();
        prepare_call_response res = fut.get();
        for (auto elem : res)
            std::cout << elem.first << " " << elem.second << std::endl;
    }

    t1.join();
    return 0;
}
