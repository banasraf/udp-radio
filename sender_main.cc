#include <iostream>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include "audio-transmission.h"
#include "udp.h"
#include "sender.h"

using namespace std;

static const uint16_t MIN_PORT = 1;
static const uint16_t MAX_PORT = std::numeric_limits<uint16_t>::max();

uint16_t validatePort(const string &port) {
    unsigned long ul = stoul(port);
    if (ul >= MIN_PORT && ul <= MAX_PORT) {
        return (uint16_t) ul;
    } else {
        throw std::exception();
    }
}

void getConfiguration(int argc, char *argv[]) {
    string mcast;
    string data_port;
    string control_port;
    string psize;
    string fsize;
    string rtime;
    string name;
    int opt;
    while ((opt = getopt(argc, argv, ":a:P:C:p:f:R:n")) != -1) {
        switch (opt) {
            case 'a':
                mcast = optarg;
                break;
            case 'P':
                data_port = optarg;
                break;
            case 'C':
                control_port = optarg;
                break;
            case 'p':
                psize = optarg;
                break;
            case 'f':
                fsize = optarg;
                break;
            case 'R':
                rtime = optarg;
                break;
            case 'n':
                name = optarg;
                break;
            default: {
                std::cout << "Invalid arguments." << std::endl;
                exit(1);
            }
        }
    }
    try {
        configuration() = std::make_unique<Configuration>(mcast);
        if (!data_port.empty()) {
            configuration()->data_address.setPort(validatePort(data_port));
        }
        if (!control_port.empty()) {
            configuration()->control_port = validatePort(control_port);
        }
        if (!psize.empty()) {
            configuration()->psize = stoul(psize);
        }
        if (!fsize.empty()) {
            configuration()->fsize = stoul(fsize);
        }
        if (!rtime.empty()) {
            configuration()->rtime = stol(rtime);
            if (configuration()->rtime <= 0) throw std::exception();
        }
        if (!name.empty()) {
            configuration()->name = name;
        }
    } catch (...) {
        std::cout << "Invalid arguments." << std::endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {

    getConfiguration(argc, argv);
    packet_fifo() =
            std::make_unique<PacketFifo>(configuration()->fsize / configuration()->psize, configuration()->psize);

    std::thread controller_thread(controller);
    std::thread resender_thread(resender);
    streamer();
    controller_thread.join();
    resender_thread.join();
    return 0;
}