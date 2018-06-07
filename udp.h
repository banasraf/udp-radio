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
        static const size_t MAX_DATAGRAM_SIZE = 2048;

    private:
        int fd;

    public:
        Socket() {
            fd = socket(AF_INET, SOCK_DGRAM, 0);
            if (fd < 0) throw SocketException("Could not create UDP socket.");
        }

        void send(const Datagram &datagram);

        Datagram receive();

        void bindToPort(uint16_t port);

        ~Socket();
    };

}

#endif //TELNET_SERVER_UDP_H
