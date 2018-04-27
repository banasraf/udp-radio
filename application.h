#ifndef TELNET_SERVER_APPLICATION_H
#define TELNET_SERVER_APPLICATION_H

#include "tcp-socket.h"
#include "telnet-processor.h"
#include "terminal.h"

const unsigned long SCREEN_SIZE = 10;

void serve_client(TcpStream &tcp_stream);

#endif //TELNET_SERVER_APPLICATION_H
