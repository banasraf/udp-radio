#ifndef TELNET_SERVER_SENDER_H
#define TELNET_SERVER_SENDER_H

#include <cstdint>
#include <vector>
#include <ctime>

#include "audio-transmission.h"
#include "udp.h"

struct Configuration {

    static const uint16_t DEFAULT_DATA_PORT = 25468;
    static const uint16_t DEFAULT_CTRL_PORT = 35468;
    static const size_t DEFAULT_PSIZE = 512;
    static const size_t DEFAULT_FSIZE = 128 * 1024;
    static const uint32_t DEFAULT_RTIME = 250;

    uint16_t control_port;
    udp::Address data_address;
    size_t psize;
    size_t fsize;
    long rtime;
    std::string name;
    uint64_t session_id;

    explicit Configuration(const std::string &mcast_addr):
            control_port(DEFAULT_CTRL_PORT),
            data_address(mcast_addr, DEFAULT_DATA_PORT),
            psize(DEFAULT_PSIZE),
            fsize(DEFAULT_FSIZE),
            rtime(DEFAULT_RTIME),
            name("Nienazwany nadajnik"),
            session_id((uint64_t) std::time(nullptr)) {}

};

std::unique_ptr<Configuration> &configuration();

std::unique_ptr<PacketFifo> &packet_fifo();

using rexmit_orders_t = std::list<std::pair<uint64_t, std::string>>;

MutexValue<rexmit_orders_t> &rexmit_orders();


void streamer();

void controller();

#endif //TELNET_SERVER_SENDER_H
