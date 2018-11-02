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

static const uint16_t MIN_PORT = 1;
static const uint16_t MAX_PORT = std::numeric_limits<uint16_t>::max();

static uint16_t validatePort(const string &port) {
    unsigned long ul = stoul(port);
    if (ul >= MIN_PORT && ul <= MAX_PORT) {
        return (uint16_t) ul;
    } else {
        throw std::exception();
    }
}

void getConfiguration(int argc, char **argv) {
    string discover_address;
    string control_port;
    string ui_port;
    string bsize;
    string rtime;
    string name;

    int opt;
    while ((opt = getopt(argc, argv, "d:C:U:b:R:n:")) != -1) {
        switch (opt) {
            case 'd':
                discover_address = optarg;
                break;
            case 'C':
                control_port = optarg;
                break;
            case 'U':
                ui_port = optarg;
                break;
            case 'b':
                bsize = optarg;
                break;
            case 'R':
                rtime = optarg;
                break;
            case 'n':
                name = optarg;
                break;

            default: {
                std::cerr << "Invalid arguments." << std::endl;
                exit(1);
            }
        }
    }

    try {
        if (!discover_address.empty()) {
            udp::Address discover_validator(discover_address, 0);
            configuration().discover_ip = discover_validator.getIP();
        }
        if (!control_port.empty()) {
            configuration().control_port = validatePort(control_port);
        }
        if (!ui_port.empty()) {
            configuration().ui_port = validatePort(ui_port);
        }
        if (!bsize.empty()) {
            configuration().bsize = stoul(bsize);
        }
        if (!rtime.empty()) {
            configuration().rtime = stol(rtime);
            if (configuration().rtime <= 0) throw exception();
        }
        if (!name.empty()) {
            configuration().name = name;
        }
    } catch (...) {
        std::cerr << "Invalid arguments." << std::endl;
        exit(1);
    }
}


int main(int argc, char **argv) {

    getConfiguration(argc, argv);

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
    auto data_output = async(dataOutput);
    auto packets_manager = async(missingPacketsManager);
    eventLoop();
    return 0;
}