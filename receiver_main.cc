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


    configuration().control_port = 10001;
    configuration().ui_port = 13333;


    auto menu_server_handle = async([](){
        while (true) {
            try {
                menuServer(configuration().ui_port);
            } catch (const SocketException &sock_exc) {
                std::cerr << sock_exc.what() << std::endl;
                exit(1);
            } catch (const IOException &io_exc) {
                std::cerr << io_exc.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // server reincarnation after failure
                continue;
            }
        }
    });
    auto discoverer_handle = async(discoverer);
    eventLoop();
    return 0;
}