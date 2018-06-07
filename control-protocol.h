#ifndef TELNET_SERVER_CONTROL_PROTOCOL_H
#define TELNET_SERVER_CONTROL_PROTOCOL_H

#include <string>

namespace ctrl {
    static const std::string LOOKUP_HEADER = "ZERO_SEVEN_COME_IN";

    static const std::string REXMIT_HEADER = "LOUDER_PLEASE";

    static const std::string REPLY_HEADER = "BOREWICZ_HERE";
}

#endif //TELNET_SERVER_CONTROL_PROTOCOL_H
