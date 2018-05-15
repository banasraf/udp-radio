#include "application.h"
#include <iostream>

using namespace std;


int telnetServer(uint16_t port) {
    try {
        TcpListener tcpListener(port);
        while (true) {
            auto net_stream = tcpListener.acceptClient();
            cout << "Client has connected from address " << net_stream.ip() << endl;
            try {
                serve_client(net_stream);
            } catch (IOException &io_exception) {
                cout << "Disconnecting client due to IO error." << endl;
                cerr << "IOException: " << io_exception.what() << endl;
            }
        }
    } catch (SocketException &socket_exception) {
        cout << "Exiting due to socket error." << endl;
        cerr << "SocketException: " << socket_exception.what() << endl;
        return 1;
    }
}


int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "USAGE: <program> port" << endl;
        return 1;
    }
    int port_i;
    try {
        port_i = std::stoi(argv[1]);
    } catch (invalid_argument &exc) {
        cout << "Invalid port argument" << endl;
        return 1;
    } catch (out_of_range &exc) {
        cout << "Port number must be between 0 and " << std::numeric_limits<uint16_t>::max() << endl;
        return 1;
    }
    if (port_i < 0 || port_i > std::numeric_limits<uint16_t>::max()) {
        cout << "Port number must be between 0 and " << std::numeric_limits<uint16_t>::max() << endl;
        return 1;
    }
    auto port = (uint16_t) port_i;
    return telnetServer(port);
}


