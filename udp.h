#ifndef TELNET_SERVER_UDP_H
#define TELNET_SERVER_UDP_H

#include <stdint-gcc.h>
#include <netinet/in.h>
#include <memory>
#include <arpa/inet.h>

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
            address->sin_port = port;
        }

        explicit Address(sockaddr_in addr): address(new sockaddr_in) {
            *address = addr;
        }

        const sockaddr_in *ptr() const;

    };

    

}

#endif //TELNET_SERVER_UDP_H
