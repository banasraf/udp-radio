#include "control-protocol.h"


bool ctrl::controlPacketBytesValidation(const std::vector<uint8_t> &bytes) {
    return std::all_of(bytes.begin(), bytes.end(), [](uint8_t byte) {
        return (byte >= 32 && byte <= 127) || byte == 10 || byte == 13;
    });
}
