#include "system-stream.h"

using namespace std;

ByteStream::series_t SystemStream::readBytes(size_type size_limit) {
    series_t series(size_limit);
    ssize_t read_count = read(in_fd, series.data(), size_limit);
    if (read_count < 0) {
        throw IOException("Error while reading");
    } else if (read_count == 0) {
        throw Eof();
    }

    series.resize((unsigned long) read_count);
    return series;
}

void SystemStream::writeBytes(const ByteStream::series_t &series) {
    size_type bytes_left = series.size();
    auto series_cursor = series.begin();
    while (bytes_left > buffer_size - buffer_cursor) {
        copy(series_cursor, series_cursor + buffer_size - buffer_cursor, output_buffer + buffer_cursor);
        bytes_left -= buffer_size - buffer_cursor;
        buffer_cursor = buffer_size;
        flushOutput();
    }
    copy(series_cursor, series.end(), output_buffer + buffer_cursor);
    buffer_cursor += distance(series_cursor, series.end());
}

void SystemStream::flushOutput() {
    ssize_t sent_count = write(out_fd, output_buffer, buffer_cursor);
    if (sent_count != (ssize_t) buffer_cursor) {
        throw IOException("Error while writing.");
    }
    buffer_cursor = 0;
}

uint8_t SystemStream::readByte() {
    uint8_t byte;
    ssize_t count = read(in_fd, &byte, 1);
    if (count < 0) {
        throw IOException("Error while reading.");
    } else if (count == 0) {
        throw Eof();
    }
    return byte;
}

void SystemStream::writeByte(uint8_t byte) {
    output_buffer[buffer_cursor] = byte;
    ++buffer_cursor;
    if (buffer_cursor == buffer_size) {
        flushOutput();
    }
}

SystemStream::~SystemStream() {
    delete [] output_buffer;
}
