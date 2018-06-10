#ifndef TELNET_SERVER_UDP_H
#define TELNET_SERVER_UDP_H

#include <netinet/in.h>
#include <memory>
#include <arpa/inet.h>
#include <vector>
#include <cstdint>
#include <unistd.h>

#include "errors.h"

namespace udp {

    class Address {

    public:
        class AddressError: public std::runtime_error {

        public:
            explicit AddressError(const std::string &message): runtime_error(message) {}

        };

    private:

        std::unique_ptr<sockaddr_in> address;

    public:
        Address(const std::string &ip, uint16_t port): address(new sockaddr_in) {
            address->sin_family = AF_INET;
            int err = inet_aton(ip.c_str(), &address->sin_addr);
            if (err == 0) throw AddressError("Wrong IP string.");
            address->sin_port = htons(port);
        }

        explicit Address(sockaddr_in addr): address(new sockaddr_in) {
            *address = addr;
        }

        Address(const Address &other): address(new sockaddr_in) {
            *address = *other.address;
        }

        Address &operator=(const Address &rhs);

        bool operator==(const Address &rhs) const {
            return rhs.address->sin_port == address->sin_port && rhs.getIP() == getIP();
        }

        const sockaddr_in *ptr() const;

        void setPort(uint16_t new_port);

        uint16_t getPort() const;

        std::string getIP() const;

    };

    struct Datagram {

        Address address;
        std::vector<uint8_t> data;

        Datagram(const Address &address, std::vector<uint8_t> data): address(address), data(std::move(data)) {}

        Datagram(const Address &address, const std::string &data_string):
                address(address),
                data(data_string.begin(), data_string.end()) {}

    };

    class Socket {

    public:
        static const size_t MAX_DATAGRAM_SIZE = 8192;

    private:
        int fd;

    public:
        Socket() {
            fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd < 0) throw SocketException("Could not create UDP socket.");
        }

        void send(const Datagram &datagram);

        Datagram receive();

        Datagram receive(long timeout);

        void bindToPort(uint16_t port);

        ~Socket();
    };

    class Broadcaster {

        static const int TTL_VALUE = 4;

        int fd;
        Address address;

    public:
        explicit Broadcaster(const Address &address): fd(socket(AF_INET, SOCK_DGRAM, 0)), address(address) {
            if (fd < 0) throw SocketException("Could not create UDP socket.");
            int optval  = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (void*)&optval, sizeof optval) < 0)
                throw SocketException("Error in setsockopt - broadcast.");
            optval = TTL_VALUE;
            if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&optval, sizeof optval) < 0)
                throw SocketException("Error  in setsockopt - multicast ttl.");

        }

        void send(const std::vector<uint8_t> &bytes);

        void send(const std::string &str);

        Datagram receive();

        Datagram receive(long timeout);

    };

    class GroupReceiver {

        int fd;
        Address address;

    public:
        explicit GroupReceiver(const Address &address): fd(socket(AF_INET, SOCK_DGRAM, 0)), address(address) {
            if (fd < 0) throw SocketException("Could not create UDP socket.");
            ip_mreq mreq;
            sockaddr_in local_address;
            mreq.imr_interface.s_addr = htonl(INADDR_ANY);
            inet_aton(address.getIP().c_str(), &mreq.imr_multiaddr);
//            mreq.imr_multiaddr = address.ptr()->sin_addr;
            if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(ip_mreq)) < 0)
                throw SocketException("Error in setsockopt - add membership.");
            local_address.sin_family = AF_INET;
            local_address.sin_addr.s_addr = htonl(INADDR_ANY);
            local_address.sin_port = address.ptr()->sin_port;
            if (bind(fd, (struct sockaddr *)&local_address, sizeof local_address) < 0)
                throw SocketException("Error in bind.");

        }

        std::vector<uint8_t> receive();

    };

}

#endif //TELNET_SERVER_UDP_H
