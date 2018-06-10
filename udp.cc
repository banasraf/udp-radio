#include <iostream>
#include "udp.h"

const sockaddr_in *udp::Address::ptr() const {
    return address.get();
}

void udp::Address::setPort(uint16_t new_port) {
    address->sin_port = htons(new_port);
}

uint16_t udp::Address::getPort() const {
    return ntohs(address->sin_port);
}

std::string udp::Address::getIP() const {
    return std::string(inet_ntoa(address->sin_addr));
}

udp::Address &udp::Address::operator=(const udp::Address &rhs) {
    if (this != &rhs) {
        *address = *rhs.address;
    }
    return *this;
}

udp::Socket::~Socket() {
    close(fd);
}

void udp::Socket::send(const udp::Datagram &datagram) {
    sendto(fd,
           datagram.data.data(),
           datagram.data.size(),
           0,
           (const sockaddr*) datagram.address.ptr(),
           sizeof(sockaddr_in));
}

udp::Datagram udp::Socket::receive() {
    std::vector<uint8_t> message(MAX_DATAGRAM_SIZE);
    sockaddr_in addr;
    socklen_t sock_size = sizeof(sockaddr_in);
    ssize_t count = recvfrom(fd, message.data(), MAX_DATAGRAM_SIZE, 0, (sockaddr*) &addr, &sock_size);
    if (count < 0) {
        throw IOException("Error in receive.");
    }
    message.resize((unsigned long) count);
    return Datagram(Address(addr), message);
}

void udp::Socket::bindToPort(uint16_t port) {
    sockaddr_in local_address;
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(port);
    if (bind(fd, (struct sockaddr *)&local_address, sizeof local_address) < 0)
        throw SocketException("Error while binding socket to port.");
}

udp::Datagram udp::Socket::receive(long timeout_ms) {
    timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = timeout_ms % 1000 * 1000;
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    int sel = select(FD_SETSIZE, &set, nullptr, nullptr, &timeout);
    if (sel > 0) return receive();
    return Datagram(udp::Address(sockaddr_in()), "");
}

void udp::Broadcaster::send(const std::vector<uint8_t> &bytes) {
    sendto(fd,
           bytes.data(),
           bytes.size(),
           0,
           (const sockaddr*) address.ptr(),
           sizeof(sockaddr_in));
}

udp::Datagram udp::Broadcaster::receive() {
    std::vector<uint8_t> message(Socket::MAX_DATAGRAM_SIZE);
    sockaddr_in addr;
    socklen_t sock_size = sizeof(sockaddr_in);
    ssize_t count = recvfrom(fd, message.data(), Socket::MAX_DATAGRAM_SIZE, 0, (sockaddr*) &addr, &sock_size);
    if (count < 0) {
        throw IOException("Error in receive.");
    }
    message.resize((unsigned long) count);
    return Datagram(Address(addr), message);
}

void udp::Broadcaster::send(const std::string &str) {
    sendto(fd,
           str.data(),
           str.size(),
           0,
           (const sockaddr*) address.ptr(),
           sizeof(sockaddr_in));
}

udp::Datagram udp::Broadcaster::receive(long timeout_ms) {
    timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = timeout_ms % 1000 * 1000;
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    int sel = select(FD_SETSIZE, &set, nullptr, nullptr, &timeout);
    if (sel > 0) return receive();
    return Datagram(udp::Address(sockaddr_in()), "");
}

std::vector<uint8_t> udp::GroupReceiver::receive() {
    std::vector<uint8_t> buffer(Socket::MAX_DATAGRAM_SIZE);
    ssize_t count = read(fd, buffer.data(), Socket::MAX_DATAGRAM_SIZE);
    if (count < 0)
        throw IOException("Error while reading from group address.");
    buffer.resize((unsigned long) count);
    return buffer;
}