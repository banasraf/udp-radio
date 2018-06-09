#include <iostream>
#include <future>
#include <queue>
#include <unistd.h>
#include <arpa/inet.h>
#include "threading.h"
#include "radio-menu.h"
#include "udp.h"
#include "audio-transmission.h"
#include "player.h"

using namespace std;

static uint64_t htonl64(uint64_t uint64) {
    if (htonl(1) == 1) return uint64;
    uint64_t result = 0;
    uint32_t tmp;
    tmp = (uint32_t) uint64;
    tmp = htonl(tmp);
    result = result | (((uint64_t) tmp) << 32u);
    tmp = (uint32_t) (uint64 >> 32u);
    tmp = htonl(tmp);
    result = result | ((uint64_t) tmp);
    return result;
}


int main(int argc, char **argv) {


//    auto menu_server_handle = async([](){ menuServer(10001); });
//    eventLoop();


//    udp::GroupReceiver gr(udp::Address("239.10.11.12", 15000));
//    auto msg = gr.receive();
//    std::cout << std::string(msg.begin(), msg.end()) << std::endl;

    udp::Address channel("239.10.11.12", 15000);
    current_channel().lock().get() = channel;
    std::thread t([=]() {dataListener(channel);});
    dataOutput();
    return 0;
}