#ifndef TELNET_SERVER_APPLICATION_H
#define TELNET_SERVER_APPLICATION_H

#include "tcp-socket.h"
#include "telnet-processor.h"
#include "terminal.h"

const unsigned long SCREEN_SIZE = 10;

class InteractiveCmdHandler: public telnet::CommandHandler {

public:
    cmd_sequence_t init() override {
        return {
                telnet::Command::Will(telnet::OPT_ECHO),
                telnet::Command::Do(telnet::OPT_LINEMODE)
        };
    }

    cmd_sequence_t response(const telnet::Command &command) override {
        return cmd_sequence_t();
    }

};

void serve_client(TcpStream &tcp_stream);

#endif //TELNET_SERVER_APPLICATION_H
