/**
 * \author: Rafal Banas
 */

#ifndef TELNET_SERVER_SYSTEM_STREAM_H
#define TELNET_SERVER_SYSTEM_STREAM_H

#include "byte-stream.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <stdexcept>

/// Exception thrown by IO methods
class IOException: public std::runtime_error {

public:
    explicit IOException(const std::string &msg): std::runtime_error(msg) {}

};

/// Abstract byte stream class for handling system input and output working on file descriptors.
class SystemStream: public ByteStream {

protected:
    int in_fd;
    int out_fd;
    size_type buffer_size;
    size_type buffer_cursor;
public:
    uint8_t *output_buffer;

    /**
     * @param in_fd - file descriptor for input
     * @param out_fd - file descriptor for ouput
     * @param buffer_size - size of output buffer
     */
    SystemStream(int in_fd, int out_fd, size_type buffer_size)
            : in_fd(in_fd),
              out_fd(out_fd),
              buffer_size(buffer_size),
              buffer_cursor(0),
              output_buffer(new uint8_t[buffer_size]) {};

    SystemStream(SystemStream &&stream)
            : in_fd(stream.in_fd),
              out_fd(stream.out_fd),
              buffer_size(stream.buffer_size),
              buffer_cursor(stream.buffer_cursor),
              output_buffer(stream.output_buffer) {
        stream.output_buffer = nullptr;
    }

public:
    series_t readBytes(size_t size_limit) override;

    uint8_t readByte() override;

    void writeBytes(const series_t &series) override;

    void writeByte(uint8_t byte) override;

    void flushOutput() override;

    /**
     * Class does not own file descriptors, so they will not be closed in destructor
     */
    virtual ~SystemStream();

};

#endif //TELNET_SERVER_SYSTEM_STREAM_H
