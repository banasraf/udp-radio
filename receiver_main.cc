#include <iostream>
#include <future>
#include <queue>
#include <unistd.h>
#include <arpa/inet.h>
#include "threading.h"
#include "radio-menu.h"
#include "udp.h"

using namespace std;

uint64_t htonl64(uint64_t uint64) {
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

int main() {

//    int sock_fd;
//    const char *host_name = "224.0.1.1";
//    auto port = (uint16_t) 12346;
//    struct sockaddr_in local_address;
//    struct ip_mreq ip_mreq;
//    char buffer[255];
//
//    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
//
//    ip_mreq.imr_interface.s_addr = htonl(INADDR_ANY);
//    if (inet_aton(host_name, &ip_mreq.imr_multiaddr) == 0)
//        perror("inet_aton");
//    if (setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&ip_mreq, sizeof ip_mreq) < 0)
//        perror("setsockopt");
//
//    local_address.sin_family = AF_INET;
//    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
//    local_address.sin_port = htons(port);
//    if (bind(sock_fd, (struct sockaddr *)&local_address, sizeof local_address) < 0)
//        perror("bind");
//
//    ssize_t count = 0;
//    do {
//        write(1, buffer, count);
//        cout << "\n";
//        count = readPacket(sock_fd, buffer, 255);
//    } while (count > 0);
//    close(sock_fd);

//    auto menu_server_handle = async([](){ menuServer(10001); });
//    eventLoop();


    udp::Socket sock;
    sock.bindToPort(10001);
    while (true) {
        auto dgram = sock.receive();
        std::cout << "Received" << std::endl;
        std::cout << string(dgram.data.begin(), dgram.data.end());
    }

    return 0;
}