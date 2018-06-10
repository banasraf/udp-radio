#ifndef TELNET_SERVER_PLAYER_H
#define TELNET_SERVER_PLAYER_H

#include "threading.h"
#include "audio-transmission.h"
#include "udp.h"

#include <chrono>

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

struct Configuration {

    const uint16_t DEFAULT_CONTROL_PORT = 35468;
    const uint16_t DEFAULT_UI_PORT = 15468;
    const size_t DEFAULT_BSIZE = 64 * 1024;
    const long DEFAULT_RTIME = 250;

    uint16_t control_port;
    std::string discover_ip;
    uint16_t ui_port;
    size_t bsize;
    long rtime;
    std::string name;

    Configuration():
            control_port(DEFAULT_CONTROL_PORT),
            discover_ip("255.255.255.255"),
            ui_port(DEFAULT_UI_PORT),
            bsize(DEFAULT_BSIZE),
            rtime(DEFAULT_RTIME),
            name() {}

};

struct RadioStation {

    udp::Address channel;
    std::string name;
    unsigned lookups;

    RadioStation(const udp::Address &channel, std::string name):
            channel(channel),
            name(std::move(name)),
            lookups(0) {}

};

MutexValue<std::list<RadioStation>> &stations();

SessionInfo &session_info();

MutexValue<std::optional<udp::Address>> &current_channel();

Configuration &configuration();

void dataListener(const std::optional<udp::Address> &channel);

void discoverer();

void dataOutput();

void missingPacketsManager();

#endif //TELNET_SERVER_PLAYER_H
