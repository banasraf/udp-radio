/**
 * \author: Rafal Banas
 */
#include "tcp-socket.h"
#include <arpa/inet.h>

using namespace std;

TcpListener::TcpListener(uint16_t port, int queue_length): port(port) {
    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        throw SocketException("Error in socket(..)");
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
        throw SocketException("Error in bind(..)");
    }

    if (listen(sock_fd, queue_length) < 0) {
        throw SocketException("Error  in listen(..)");
    }
}

TcpStream TcpListener::acceptClient() {
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int incoming_sock_fd = accept(sock_fd, (struct sockaddr *) &client_address, &client_address_len);
    if (incoming_sock_fd < 0) {
        throw SocketException("Error in accept(..)");
    }

    return TcpStream(incoming_sock_fd, client_address, TCP_BUFFER_SIZE);
}

std::string TcpStream::ip() {
    return std::string(inet_ntoa(client_address.sin_addr));
}

TcpListener::~TcpListener() noexcept {
    close(sock_fd);
}

TcpStream::~TcpStream() noexcept {
    close(in_fd);
}
