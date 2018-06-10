#ifndef TELNET_SERVER_AUDIO_TRANSMISSION_H
#define TELNET_SERVER_AUDIO_TRANSMISSION_H

#include <cstdint>
#include <vector>
#include <optional>
#include <map>
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

    static std::optional<AudioPacket> fromBytes(const std::vector<uint8_t> &bytes);

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

class AudioBuffer {

    using packets_t = std::list<std::optional<AudioPacket>>;

    size_t size;
    size_t psize;
    packets_t packets;
    std::map<uint64_t, packets_t::iterator> missing_map;
    MutexValue<std::list<uint64_t>> &missing_packets;
    uint64_t lowest_fbn;
    bool ready;
    uint64_t last_fbn;

    void dropPacket(LockedValue<std::list<uint64_t>> &_lock);

    void insertPacket(const std::optional<AudioPacket> &ap, LockedValue<std::list<uint64_t>> &_lock);

    void putMissing(const AudioPacket &ap);

public:
    AudioBuffer(size_t size, const AudioPacket &initial_packet, MutexValue<std::list<uint64_t>> &missing_packets):
            size(size),
            psize(initial_packet.data.size()),
            packets({initial_packet}),
            missing_packets(missing_packets),
            lowest_fbn(initial_packet.first_byte_num),
            ready(false), last_fbn(initial_packet.first_byte_num) {}

    void push(const AudioPacket &ap);

    std::optional<AudioPacket> pop();

    bool isReady();

};

#endif //TELNET_SERVER_AUDIO_TRANSMISSION_H
