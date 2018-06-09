#include <iostream>
#include "player.h"

size_t BUFFER_SIZE = 1024;

SessionInfo &session_info() {
    static auto *session = new SessionInfo;
    return *session;
}

static void putPacket(const AudioPacket &packet) {
    if (session_info().psize.lock().get() == packet.data.size()) {
        session_info().buffer.lock().get()->push(packet);
    }
}

static void resetSession(const AudioPacket &initial_packet) {
    session_info().missing_packets.lock()->clear();
    session_info().buffer.lock().get() =
            std::make_unique<AudioBuffer>(BUFFER_SIZE, initial_packet, session_info().missing_packets);
    session_info().session_id.lock().get() = initial_packet.session_id;
    session_info().psize.lock().get() = initial_packet.data.size();
    session_info().initiated.lock().get() = true;
}

static bool channelIsCurrent(const udp::Address &channel) {
    auto _lock = current_channel().lock();
    return _lock->has_value() && *_lock.get() == channel;
}

void dataListener(const udp::Address &channel) {
    udp::GroupReceiver receiver(channel);
    while (channelIsCurrent(channel)) {
        auto message = receiver.receive();
        auto packet_op = AudioPacket::fromBytes(message);
        if (!packet_op) continue;
        auto packet = *packet_op;
        if (session_info().initiated.lock().get()) {
            if (packet.session_id < session_info().session_id.lock().get()) continue;
            if (packet.session_id == session_info().session_id.lock().get()) {
                putPacket(packet);
            } else {
                session_info().initiated.lock().get() = false;
                resetSession(packet);
            }
        } else {
            resetSession(packet);
        }
    }
}

void dataOutput() {
    while (true) {
        if (session_info().ready()) {
            auto packet = session_info().buffer.lock().get()->pop();
            if (packet) {
                write(STDOUT_FILENO, packet->data.data(), packet->data.size());
            } else {
                session_info().initiated.lock().get() = false;
            }
        }
    }
}

MutexValue<std::optional<udp::Address>> &current_channel() {
    static auto *value = new MutexValue(std::optional<udp::Address>({}));
    return *value;
}

bool SessionInfo::ready() {
    return session_info().initiated.lock().get() && session_info().buffer.lock().get()->isReady();
}
