#ifndef TELNET_SERVER_AUDIO_TRANSMISSION_H
#define TELNET_SERVER_AUDIO_TRANSMISSION_H

#include <cstdint>
#include <vector>
#include <optional>
#include "threading.h"

struct AudioPacket {
    uint64_t session_id;
    uint64_t first_byte_num;
    std::vector<uint8_t> data;

    AudioPacket(uint64_t session_id, uint64_t first_byte_num, std::vector<uint8_t> data):
            session_id(session_id),
            first_byte_num(first_byte_num),
            data(std::move(data)) {}

    AudioPacket(): session_id(0), first_byte_num(0), data() {};
    
    std::vector<uint8_t> toBytes();

};

class PacketReader {
    uint64_t session_id;
    uint64_t fbn;
    uint64_t psize;

    uint8_t *buffer;
    uint64_t cursor;

public:
    PacketReader(uint64_t psize, uint64_t session_id):
            session_id(session_id),
            fbn(0),
            psize(psize),
            buffer(new uint8_t[psize]),
            cursor(0) {}

    std::optional<AudioPacket> readPacket();

    ~PacketReader();

};

class PacketFifo {

    unsigned long size;
    unsigned long psize;
    MutexValue<std::vector<AudioPacket>> data;
    unsigned long cursor;
    uint64_t last_fbn; // last packet's first_byte_num

public:
    PacketFifo(unsigned long size, unsigned long psize):
            size(size),
            psize(psize),
            data(std::vector<AudioPacket>(size)),
            cursor(0),
            last_fbn(0) {}

    void push(const AudioPacket &ap);

    bool hasPacket(uint64_t fbn);

    std::optional<AudioPacket> getPacket(uint64_t fbn);

};

#endif //TELNET_SERVER_AUDIO_TRANSMISSION_H
