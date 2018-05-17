#ifndef TELNET_SERVER_THREADING_H
#define TELNET_SERVER_THREADING_H

#include "byte-stream.h"

#include <mutex>
#include <future>
#include <queue>
#include <list>

template <typename T>
class MutexValue;

template <typename T>
class LockedValue {

    std::lock_guard<std::mutex> guard;
    T &value;

public:
    LockedValue(std::mutex &value_mutex, T &value): guard(value_mutex), value(value) {}

    LockedValue(MutexValue<T> &mv): guard(mv.value_mutex), value(mv.value) {}

    T &get() {
        return value;
    }

    T *operator->() {
        return &value;
    }

};

template <typename T>
class MutexValue {

    friend class LockedValue<T>;

    T value;
    std::mutex value_mutex;

public:
    explicit MutexValue(const T &value): value(value) {}

    MutexValue(): value() {};

    LockedValue<T> lock() {
        return LockedValue<T>(value_mutex, value);
    }

};

/**
 * Class for collecting input from multiple threads.
 * @tparam T
 */
template <typename T>
class MultiInputStream {

    MutexValue<std::queue<T>> t_queue;
    std::mutex access_mutex;
    std::condition_variable cv;

public:
    /**
     * Write v to stream
     * @param v
     */
    void write(const T &v) {
        t_queue.lock().get().push(v);
        cv.notify_one();
    }

    /**
     * Read value from stream. Blocks thread until there is something to read.
     */
    T read() {
        std::unique_lock<std::mutex> _lock(access_mutex);
        cv.wait(_lock, [&](){ return !t_queue.lock().get().empty(); });
        auto queue_lock = t_queue.lock();
        T v = queue_lock.get().front();
        queue_lock.get().pop();
        _lock.unlock();
        return v;
    }

};


class ByteStreamWriter {

    std::shared_ptr<ByteStream> stream;

public:
    void writeBytes(const ByteStream::series_t &series);

    void writeByte(uint8_t byte);

    void flushOutput();

    explicit ByteStreamWriter(const std::shared_ptr<ByteStream> &stream): stream(stream) {}

};

class LockedMultiWriter {

    std::list<LockedValue<ByteStreamWriter>> _locks;

public:
    std::list<LockedValue<ByteStreamWriter>> &locks();

    void writeBytes(ByteStream::series_t bytes);

    void writeByte(uint8_t byte);

    void flushOutput();

};

class MultiWriter {

    MutexValue<std::list<std::shared_ptr<MutexValue<ByteStreamWriter>>>> streams;

public:
    void addStream(const std::shared_ptr<MutexValue<ByteStreamWriter>> &stream);

    void removeStream(const std::shared_ptr<MutexValue<ByteStreamWriter>> &stream);

    LockedMultiWriter lock();

};

#endif //TELNET_SERVER_THREADING_H
