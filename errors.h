#ifndef TELNET_SERVER_ERRORS_H
#define TELNET_SERVER_ERRORS_H

#include <stdexcept>

/// Exception thrown by IO methods
class IOException: public std::runtime_error {

public:
    explicit IOException(const std::string &msg): std::runtime_error(msg) {}

};

/// Exception thrown when socket error occurs
class SocketException: public std::runtime_error {

public:
    explicit SocketException(const std::string &msg): std::runtime_error(msg) {}

};

#endif //TELNET_SERVER_ERRORS_H
