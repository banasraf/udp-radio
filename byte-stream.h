/**
 * \author: Rafal Banas
 */

#ifndef TELNET_SERVER_BYTE_STREAM_H
#define TELNET_SERVER_BYTE_STREAM_H

#include <cstdint>
#include <vector>

class Eof: public std::exception {};

/**
 * Bidirectional byte stream with buffered output interface
 */
class ByteStream {

public:
    using series_t = std::vector<uint8_t>;
    using size_type = unsigned long;

    /**
     * Read one byte from stream
     */
    virtual uint8_t readByte() = 0;

    /**
     * Read series of bytes
     * @param count - maximum size of series to read
     */
    virtual series_t readBytes(size_type count) = 0;

    /**
     * Write one byte to buffer
     * @param byte - byte to write
     */
    virtual void writeByte(uint8_t byte) = 0;

    /**
     * Write series of bytes to buffer
     * @param series
     */
    virtual void writeBytes(const series_t &series) = 0;

    /**
     * Send the content of output buffer
     */
    virtual void flushOutput() = 0;

};

#endif //TELNET_SERVER_BYTE_STREAM_H
