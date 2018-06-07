#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include "audio-transmission.h"

static uint64_t htonl64(uint64_t uint64) {
    if (htonl(1) == 1) return uint64;
    uint64_t result = 0;
    uint32_t tmp;
    tmp = (uint32_t) uint64;
    tmp = htonl(tmp);
    result = result | (((uint64_t) tmp) << 32u);
    tmp = (uint32_t) (uint64 >> 32u);
    tmp = htonl(tmp);
    result = result | ((uint64_t) tmp);
    return result;
}

void PacketFifo::push(const AudioPacket &ap) {
    auto lock = data.lock();
    lock.get()[cursor] = ap;
    cursor = (cursor + 1) % size;
    last_fbn = ap.first_byte_num;
}

std::optional<AudioPacket> PacketFifo::getPacket(uint64_t fbn) {
    auto lock = data.lock();
    if (!hasPacket(fbn)) return {};
    unsigned long packet_index = (size + cursor - 1 - (last_fbn - fbn) / psize) % size;
    return std::make_optional(lock.get()[packet_index]);
}

bool PacketFifo::hasPacket(uint64_t fbn) {
    return fbn <= last_fbn && fbn > last_fbn - size * psize;
}

std::optional<AudioPacket> PacketReader::readPacket() {
    while (cursor < psize - 1) {
        ssize_t count = read(0, buffer + cursor, psize - cursor);
        if (count > 0) {
            cursor += count;
        } else if (count == 0) {
            return {};
        } else {
            throw std::runtime_error("Input error.");
        }
    }
    auto result = AudioPacket(session_id, fbn, std::vector<uint8_t>(buffer, buffer + psize));
    fbn += psize;
    cursor = 0;
    return std::make_optional(result);
}

PacketReader::~PacketReader() {
    delete[] buffer;
}

std::vector<uint8_t> AudioPacket::toBytes() {
    std::vector<uint8_t> result(data.size() + 16);
    auto session_n = htonl64(session_id);
    std::memcpy(result.data(), &session_n, 8);
    auto fbn_n = htonl64(first_byte_num);
    std::memcpy(result.data() + 8, &fbn_n, 8);
    std::memcpy(result.data() + 16, data.data(), data.size());
    return result;
}
