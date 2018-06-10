#include <iostream>
#include <sstream>
#include "player.h"
#include "control-protocol.h"
#include "radio-menu.h"

SessionInfo &session_info() {
    static auto *session = new SessionInfo;
    return *session;
}

Configuration &configuration() {
    static auto *conf = new Configuration;
    return *conf;
}

static void putPacket(const AudioPacket &packet) {
    if (session_info().psize.lock().get() == packet.data.size()) {
        std::cerr << packet.first_byte_num << std::endl;
        session_info().buffer.lock().get()->push(packet);
    }
}

static void resetSession(const AudioPacket &initial_packet) {
    session_info().missing_packets.lock()->clear();
    session_info().buffer.lock().get() = std::make_unique<AudioBuffer>(
            configuration().bsize / initial_packet.data.size(),
            initial_packet,
            session_info().missing_packets
    );
    session_info().session_id.lock().get() = initial_packet.session_id;
    session_info().psize.lock().get() = initial_packet.data.size();
    session_info().initiated.lock().get() = true;
}

static bool channelIsCurrent(const udp::Address &channel) {
    bool result;
    {
        auto _lock = current_channel().lock();
        result =  _lock->has_value() && *_lock.get() == channel;
    }
    return result;
}

void dataListener(const std::optional<udp::Address> &_channel) {
    if (!_channel)
        return;
    udp::Address channel = *_channel;
    std::cerr << "listener " << channel.getIP() << ":" << channel.getPort() << std::endl;
    udp::GroupReceiver receiver(channel);
    std::cerr << "receiver created " << channel.getIP() << ":" << channel.getPort() << std::endl;
    while (channelIsCurrent(channel)) {
        auto message = receiver.receive(100);
        if (message.empty()) continue;
        std::cerr << "received" << std::endl;
        auto packet_op = AudioPacket::fromBytes(message);
        if (!packet_op) continue;
        std::cerr << "formatted" << std::endl;
        auto packet = *packet_op;
        auto initiated = session_info().initiated.lock().get();
        if (initiated) {
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

MutexValue<std::list<RadioStation>> &stations() {
    static auto *_stations = new MutexValue<std::list<RadioStation>>(std::list<RadioStation>());
    return *_stations;
}

const unsigned MAX_LOOKUPS = 4;

void changeChannel(const std::optional<udp::Address> &address) {
    current_channel().lock().get() = address;
    session_info().initiated.lock().get() = false;

    event_stream().write(MenuEvent(ApplicationEventType::CHANGE_CHANNEL));
}

void setNewMenu(LockedValue<std::list<RadioStation>> &_lock) {
    if (_lock->empty()) {
        radio_menu().lock()->setListing(nullptr);
        event_stream().write(MenuEvent(ApplicationEventType::MENU_CHANGE));
        return;
    }
    std::vector<menu::Option> options;
    unsigned current_index = 0;
    auto ch_lock = current_channel().lock();
    unsigned i = 0;
    for (auto &rs: _lock.get()) {
        if (rs.channel == ch_lock.get()) current_index = i;
        options.emplace_back(rs.name, [=](){ changeChannel(rs.channel); });
        ++i;
    }
    options_listing().lock().get() = std::make_unique<menu::OptionsListing>(options);
    radio_menu().lock()->setListing(options_listing().lock().get().get());
    options_listing().lock().get()->active_index = current_index;
    event_stream().write(MenuEvent(ApplicationEventType::MENU_CHANGE));
}

void sendLookup(udp::Broadcaster &sock) {
    sock.send(ctrl::LOOKUP_HEADER);
    auto _lock = stations().lock();
    bool to_delete = false;
    for (auto &it : _lock.get()) {
        if (++it.lookups >= MAX_LOOKUPS) to_delete = true;
    }
    if (to_delete) {
        std::list<RadioStation> new_list;
        std::copy_if(
                _lock.get().begin(),
                _lock.get().end(),
                std::back_inserter(new_list),
                [](const RadioStation &rs) { return rs.lookups < MAX_LOOKUPS; }
                );
        _lock.get() = new_list;

        {
            auto ch_lock = current_channel().lock();
            bool deleted = !std::any_of(
                    new_list.begin(),
                    new_list.end(),
                    [&](const RadioStation &rs) { return rs.channel == ch_lock.get(); }
                    );
            if (deleted) {
                ch_lock.get() = (new_list.empty()) ? std::optional<udp::Address>() : new_list.front().channel;
                event_stream().write(MenuEvent(ApplicationEventType::CHANGE_CHANNEL));
            }
        }

        setNewMenu(_lock);
    }
}

const unsigned LOOKUP_INTERVAL_SECONDS = 5;
const long REPLY_TIMEOUT_MILLISECONDS = 200;

static uint16_t validatePort(const std::string &port) {
    unsigned long ul = stoul(port);
    if (ul >= std::numeric_limits<uint16_t>::min() && ul <= std::numeric_limits<uint16_t>::max()) {
        return (uint16_t) ul;
    } else {
        throw std::exception();
    }
}

std::optional<RadioStation> parseReply(const std::vector<uint8_t> &bytes) {
    std::stringstream ss(std::string(bytes.begin(), bytes.end()));
    std::string header;
    ss >> header;
    if (header != ctrl::REPLY_HEADER) return {};
    std::string mcast;
    std::string port_str;
    uint16_t port;
    ss >> mcast >> port_str;
    std::unique_ptr<udp::Address> addr;
    try {
        port = validatePort(port_str);
    } catch (...) {
        return {};
    }
    try {
        addr = std::make_unique<udp::Address>(mcast, port);
    } catch (...) {
        return {};
    }
    std::string name;
    std::getline(ss, name);
    if (name.size() > ctrl::MAX_NAME_LENGTH) return {};

    return RadioStation(*addr, name);
}

void removeStation(LockedValue<std::list<RadioStation>> &list_lock, const RadioStation &station) {
    list_lock->remove_if([&](const RadioStation &rs) { return rs.channel == station.channel; });
}

void insertStation(LockedValue<std::list<RadioStation>> &list_lock, const RadioStation &station, bool is_reinsert) {
    auto where_to_insert = list_lock->begin();
    while (where_to_insert != list_lock->end()) {
        if (station.name > where_to_insert->name) break;
        ++where_to_insert;
    }
    list_lock->insert(where_to_insert, station);
    if (list_lock->size() == 1 && !is_reinsert)
        changeChannel(station.channel);
    setNewMenu(list_lock);
}

void handleReply(const std::vector<uint8_t> &bytes) {
    auto station = parseReply(bytes);
    if (!station) return;
    auto list_lock = stations().lock();
    bool present = std::any_of(
            list_lock->begin(),
            list_lock->end(),
            [&](const RadioStation &rs) { return rs.channel == station->channel; }
            );
    if (present) {
        removeStation(list_lock, *station);
    }
    insertStation(list_lock, *station, present);
}

void discoverer() {
    udp::Broadcaster sock(udp::Address(configuration().discover_ip, configuration().control_port));
    while (true) {
        sendLookup(sock);
        auto timer = std::chrono::system_clock::now();
        while (std::chrono::system_clock::now() - timer < std::chrono::seconds(LOOKUP_INTERVAL_SECONDS)) {
            auto dgram = sock.receive(REPLY_TIMEOUT_MILLISECONDS);
            if (dgram.data.empty()) continue;
            if (!ctrl::controlPacketBytesValidation(dgram.data)) continue;
            handleReply(dgram.data);
        }
    }
}

std::string rexmitMessage(const std::list<uint64_t> &packets) {
    if (packets.empty()) return "";
    std::stringstream ss;
    ss << ctrl::REXMIT_HEADER << " ";
    for (auto it = packets.begin(); it != std::prev(packets.end()); ++it) {
        ss << *it << ',';
    }
    ss << packets.back() << std::endl;
    return ss.str();
}

void missingPacketsManager() {
    udp::Broadcaster sock(udp::Address(configuration().discover_ip, configuration().control_port));
    while (true) {
        std::list<uint64_t> packets_list;
        {
            auto _lock = session_info().missing_packets.lock();
            packets_list = _lock.get();
            _lock->clear();
        }
        auto message = rexmitMessage(packets_list);
        if (!message.empty()) sock.send(message);
        std::this_thread::sleep_for(std::chrono::milliseconds(configuration().rtime));
    }
}

bool SessionInfo::ready() {
    return session_info().initiated.lock().get() && session_info().buffer.lock().get()->isReady();
}
