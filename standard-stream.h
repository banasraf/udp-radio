#ifndef TELNET_SERVER_STANDARD_STREAM_H
#define TELNET_SERVER_STANDARD_STREAM_H

#include "system-stream.h"

class StdStream: public SystemStream {

public:
    StdStream(): SystemStream(0, 1, 16) {};

};

#endif //TELNET_SERVER_STANDARD_STREAM_H
