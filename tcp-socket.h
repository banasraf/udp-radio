/**
 * \author: Rafal Banas
 * Module for handling tcp connections.
 */

#ifndef TELNET_SERVER_TCPSOCKET_H
#define TELNET_SERVER_TCPSOCKET_H

#include "byte-stream.h"
#include "system-stream.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <unistd.h>
#include <vector>
#include <memory>

const int QUEUE_LENGTH = 10;
const unsigned long TCP_BUFFER_SIZE = 1024;

/// Exception thrown when socket error occurs
class SocketException: public std::runtime_error {

public:
    explicit SocketException(const std::string &msg): std::runtime_error(msg) {}

};

/// Byte stream for tcp connection. Can be constructed only by TcpListener
class TcpStream: public SystemStream {

public:
    using size_type = unsigned long;

    friend class TcpListener;

private:
    struct sockaddr_in client_address;


    explicit TcpStream(int sock_fd, struct sockaddr_in client_address, size_type buffer_size)
            : SystemStream(sock_fd, sock_fd, buffer_size),
              client_address(client_address) {}

public:
    TcpStream(TcpStream &&stream) noexcept: SystemStream(std::move(stream)), client_address(stream.client_address) {
        stream.inactivate();
    }

    std::string ip();

    void inactivate();

    ~TcpStream() noexcept override;

};

/// Listener bound to given port. Should be used for waiting for client connection
class TcpListener {

    int sock_fd;
    struct sockaddr_in server_address;
    uint16_t port;

public:
    /**
     * Constructs connections listener.
     * @param port - port to listen on
     * @param queue_length - length of requests queue, default: QUEUE_LENGTH
     */
    explicit TcpListener(uint16_t port, int queue_length = QUEUE_LENGTH);

    /**
     * Accepts connection from client.
     * @return stream to read from / write to client.
     */
    std::shared_ptr<TcpStream> acceptClient();

    ~TcpListener() noexcept;

};


#endif //TELNET_SERVER_TCPSOCKET_H
