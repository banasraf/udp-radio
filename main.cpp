#include "application.h"
#include <iostream>

using namespace std;


void telnetServer() {
    auto telnetCmdHandler = InteractiveCmdHandler();
    TcpListener tcpListener(10001);
    auto net_stream = tcpListener.acceptClient();
    serve_client(net_stream);
}


int main() {
    telnetServer();
}


