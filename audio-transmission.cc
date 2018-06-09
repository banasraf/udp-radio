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
    return fbn <= last_fbn && size * psize > last_fbn - fbn && fbn % psize == 0;
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

std::optional<AudioPacket> AudioPacket::fromBytes(const std::vector<uint8_t> &bytes) {
    if (bytes.size() < 2 * sizeof(uint64_t)) return {};
    uint64_t session_id;
    uint64_t fbn;
    memcpy(&session_id, bytes.data(), sizeof(uint64_t));
    session_id = htonl64(session_id);
    memcpy(&fbn, bytes.data() + sizeof(uint64_t), sizeof(uint64_t));
    fbn = htonl64(fbn);
    std::vector<uint8_t> data(bytes.size() - 2*sizeof(uint64_t));
    std::memcpy(data.data(), bytes.data() + 2 * sizeof(uint64_t), bytes.size() - 2 * sizeof(uint64_t));
    return std::make_optional<AudioPacket>(session_id, fbn, data);
}

void AudioBuffer::push(const AudioPacket &ap) {
    if (missing_map.find(ap.first_byte_num) != missing_map.end()) {
        putMissing(ap);
    }

    if (ap.first_byte_num <= packets.back()->first_byte_num)
        return;

    for (uint64_t fbn = packets.back()->first_byte_num + psize; fbn < ap.first_byte_num; fbn += psize) {
        insertPacket({});
        missing_map[fbn] = packets.end();
    }
    insertPacket(ap);
    if (!ready && packets.size() >= 3 * size / 4) ready = true;
}

void AudioBuffer::insertPacket(const std::optional<AudioPacket> &ap) {
    if (packets.size() == size) {
        dropPacket();
    }
    packets.push_back(ap);
}

void AudioBuffer::dropPacket() {
    missing_map.erase(lowest_fbn);
    packets.pop_front();
    missing_packets.lock()->remove(lowest_fbn);
    lowest_fbn += psize;
}

void AudioBuffer::putMissing(const AudioPacket &ap) {
    missing_packets.lock()->remove(ap.first_byte_num);
    auto it = missing_map.find(ap.first_byte_num);
    *(it->second) = ap;
}

std::optional<AudioPacket> AudioBuffer::pop() {
    if (packets.empty()) return {};
    auto packet = packets.front();
    dropPacket();
    return packet;
}

bool AudioBuffer::isReady() {
    return ready;
}
