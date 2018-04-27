#ifndef TELNET_SERVER_APPLICATION_H
#define TELNET_SERVER_APPLICATION_H

#include "tcp-socket.h"
#include "telnet-processor.h"
#include "terminal.h"

/**
 * Number of lines application can write on.
 */
const unsigned long SCREEN_SIZE = 10;

/**
 * Serve application for client connected through given tcp stream
 */
void serve_client(TcpStream &tcp_stream);

#endif //TELNET_SERVER_APPLICATION_H
