#ifndef TELNET_SERVER_PLAYER_H
#define TELNET_SERVER_PLAYER_H

#include "threading.h"
#include "audio-transmission.h"
#include "udp.h"

struct SessionInfo {

    MutexValue<std::unique_ptr<AudioBuffer>> buffer;
    MutexValue<std::list<uint64_t>> missing_packets;
    MutexValue<bool> initiated;
    MutexValue<uint64_t> session_id;
    MutexValue<size_t> psize;

    SessionInfo(): buffer(std::unique_ptr<AudioBuffer>()),
                   missing_packets(std::list<uint64_t>()),
                   initiated(false),
                   session_id(0), psize(0) {}

   bool ready();

};

SessionInfo &session_info();

MutexValue<std::optional<udp::Address>> &current_channel();

void dataListener(const udp::Address &channel);

void dataOutput();

#endif //TELNET_SERVER_PLAYER_H
