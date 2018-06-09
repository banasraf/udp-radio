#include "sender.h"
#include "control-protocol.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <unistd.h>

void streamer() {

    PacketReader pr(configuration()->psize, configuration()->session_id);
    udp::Broadcaster sock(configuration()->data_address);

    while (true) {
        auto packet = pr.readPacket();
        if (!packet) break;
        packet_fifo()->push(*packet);
        sock.send(packet->toBytes());
    }

}

std::unique_ptr<Configuration> &configuration() {
    static std::unique_ptr<Configuration> _configuration;
    return _configuration;
}

std::unique_ptr<PacketFifo> &packet_fifo() {
    static std::unique_ptr<PacketFifo> _packet_fifo;
    return _packet_fifo;
}


bool controlPacketBytesValidation(const std::vector<uint8_t> &bytes) {
    return std::all_of(bytes.begin(), bytes.end(), [](uint8_t byte) {
        return (byte >= 32 && byte <= 127) || byte == 10 || byte == 13;
    });
}

std::string reply_message() {
    std::stringstream result;
    result << ctrl::REPLY_HEADER << ' ';
    result << configuration()->data_address.getIP() << ' ' << configuration()->data_address.getPort() << ' ';
    result << configuration()->name;
    return result.str();
}

void handle_lookup(const udp::Address &caller, udp::Socket &sock) {
    sock.send(udp::Datagram(caller, reply_message()));
}

void handle_rexmit(const udp::Address &caller, const std::string &packets_list) {
    std::string fbn_string;
    std::stringstream ss(packets_list);
    rexmit_orders_t new_orders;
    while (std::getline(ss, fbn_string, ',')) {
        uint64_t fbn;
        try {
            fbn = std::stoul(fbn_string);
            if (fbn % configuration()->psize == 0) new_orders.emplace_back(fbn, caller.getIP());
        } catch (...) {}
    }

    auto orders_lock = rexmit_orders().lock();
    orders_lock->splice(orders_lock->end(), new_orders);
}

void controller() {
    udp::Socket sock;
    sock.bindToPort(configuration()->control_port);
    while (true) {
        auto datagram = sock.receive();
        if (!controlPacketBytesValidation(datagram.data))
            continue;
        auto author = datagram.address;
        auto message = std::string(datagram.data.begin(), datagram.data.end());
        std::stringstream sstream(message);
        std::string message_header;
        sstream >> message_header;
        if (message_header == ctrl::LOOKUP_HEADER) {
            handle_lookup(author, sock);
        } else if (message_header == ctrl::REXMIT_HEADER) {
            std::string packets_list;
            sstream >> packets_list;
            handle_rexmit(author, packets_list);
        } else if (message_header == "SHOW") {
            auto orders = rexmit_orders().lock().get();
            std::for_each(orders.begin(), orders.end(), [](const std::pair<uint64_t, std::string> &order) {
               std::cout << "From " << order.second << " packet " << order.first << " |" << std::endl;
            });
        }
    }
}

void resend_packets(udp::Socket &sock) {
    rexmit_orders_t orders;
    {
        auto lock = rexmit_orders().lock();
        orders = lock.get();
        lock->clear();
    }
    for (auto &order : orders) {
        auto packet = packet_fifo()->getPacket(order.first);
        if (packet) {
            udp::Datagram dgram(
                    udp::Address(order.second, configuration()->data_address.getPort()),
                    packet->toBytes()
            );
            try {
                sock.send(dgram);
            } catch (const IOException &exc) {
                std::cout << "Error occurred while resending packet" << std::endl;
                std::cerr << exc.what() << std::endl;
            }
        }
    }
}

void resender() {
    udp::Socket sock;
    timespec t {
            configuration()->rtime / 1000,
            configuration()->rtime % 1000 * 1000
    };
    timespec trem {0, 0};
    while (true) {
        resend_packets(sock);
        nanosleep(&t, &trem);
    }
}

MutexValue<rexmit_orders_t> &rexmit_orders() {
    static MutexValue<rexmit_orders_t> *orders = new MutexValue<rexmit_orders_t>(rexmit_orders_t());
    return *orders;
}
