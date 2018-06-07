#include <iostream>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <sstream>

#include "audio-transmission.h"
#include "udp.h"
#include "sender.h"

int main() {

//    const char *host_name = "224.0.1.1";
//    const uint16_t port = (uint16_t) 12346;
//    std::string message = "Bla bla bla";
//    int broadcast = 1;
//    int ttl = 4;
//
//    struct hostent *host = gethostbyname(host_name);
//
//    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
//    printf("%d\n", sock_fd);
//    if (setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) < 0) {
//        perror("setsockopt");
//        exit(1);
//    }
//
//    if (setsockopt(sock_fd, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, sizeof ttl) < 0)
//        perror("setsockopt multicast ttl");
//
//    ttl = 0;
//    if (setsockopt(sock_fd, SOL_IP, IP_MULTICAST_LOOP, (void*)&ttl, sizeof ttl) < 0)
//        perror("setsockopt loop");
//
//    struct sockaddr_in their_addr;
//
//    their_addr.sin_family = AF_INET;     // host byte order
//    their_addr.sin_port = htons(port); // short, network byte order
//    their_addr.sin_addr = *((struct in_addr *)host->h_addr);
//    std::memset(their_addr.sin_zero, 0, sizeof their_addr.sin_zero);
//
//    while (true) {
//        if (sendto(sock_fd, message.c_str(), message.size(), 0, (struct sockaddr *) &their_addr, sizeof(their_addr)) <
//            0) {
//            perror("sendto");
//            close(sock_fd);
//            exit(1);
//        }
//        sleep(2);
//    }

    configuration() = std::make_unique<Configuration>("127.0.0.1");
    packet_fifo() =
            std::make_unique<PacketFifo>(configuration()->fsize / configuration()->psize, configuration()->psize);

    configuration()->psize = 15;
    configuration()->control_port = 10001;
    controller();
//    configuration()->data_address.setPort(10001);
//    streamer();

}