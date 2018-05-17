#include "threading.h"

void ByteStreamWriter::writeBytes(const ByteStream::series_t &series) {
    stream->writeBytes(series);
}

void ByteStreamWriter::writeByte(uint8_t byte) {
    stream->writeByte(byte);
}

void ByteStreamWriter::flushOutput() {
    stream->flushOutput();
}

std::list<LockedValue<ByteStreamWriter>> &LockedMultiWriter::locks() {
    return _locks;
}

void LockedMultiWriter::writeBytes(ByteStream::series_t bytes) {
    for (auto &lock : _locks) {
        lock->writeBytes(bytes);
    }
}

void LockedMultiWriter::writeByte(uint8_t byte) {
    for (auto &lock : _locks) {
        lock->writeByte(byte);
    }
}

void LockedMultiWriter::flushOutput() {
    for (auto &lock : _locks) {
        lock->flushOutput();
    }
}

void MultiWriter::addStream(const std::shared_ptr<MutexValue<ByteStreamWriter>> &stream) {
    auto list_lock = streams.lock();
    list_lock->remove(stream);
    list_lock->push_back(stream);
}

void MultiWriter::removeStream(const std::shared_ptr<MutexValue<ByteStreamWriter>> &stream) {
    streams.lock()->remove(stream);
}

LockedMultiWriter MultiWriter::lock() {
    auto _lock = LockedMultiWriter();
    auto list_lock = streams.lock();
    for (auto &stream : list_lock.get()) {
        _lock.locks().emplace_back(*stream);
    }
    return _lock;
}
