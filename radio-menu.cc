#include <list>
#include <memory>
#include <iostream>
#include "telnet-processor.h"
#include "radio-menu.h"
#include "player.h"


MutexValue<menu::Menu> &radio_menu() {
    static auto *menu = new MutexValue<menu::Menu>(menu::Menu(nullptr));
    return *menu;
}

MultiInputStream<MenuEvent> &event_stream() {
    static auto *_event_stream = new MultiInputStream<MenuEvent>();
    return *_event_stream;
}

void handleUserInput(ByteStream &net_stream) {
    bool client_exit = false;
    terminal::KeyStream keyStream(net_stream);
    while (!client_exit) {
        auto key = keyStream.nextKey();
        if (key.isAction()) {
            switch (key.action) {
                case terminal::ActionKeyType::ARROW_DOWN:
                case terminal::ActionKeyType::ARROW_UP: {
                    event_stream().write(MenuEvent(key.action));
                    break;
                }
                case terminal::ActionKeyType::END_OF_STREAM: {
                    client_exit = true;
                }
                default:
                    break;
            }
        }
    }
}

void sendMenu() {
    auto menu_lock = radio_menu().lock();
    auto output_lock = menu_output().lock();
    auto message = terminalScreen(menuToString(menu_lock.get()));
    try {
        output_lock.writeBytes(message);
        output_lock.flushOutput();
    } catch (const IOException &e) {
        std::cerr << "Radio menu: write error." << std::endl;
    }
}

void eventLoop() {
    bool running = true;
    std::list<std::future<void>> players;
    while (running) {
        auto event = event_stream().read();
        if (event.tag == MenuEvent::Tag::USER_EVENT) {
            switch (event.key) {
                case terminal::ActionKeyType::ARROW_DOWN: {
                    radio_menu().lock()->down();
                    radio_menu().lock()->enter();
                    sendMenu();
                    break;
                }
                case terminal::ActionKeyType::ARROW_UP: {
                    radio_menu().lock()->up();
                    radio_menu().lock()->enter();
                    sendMenu();
                    break;
                }
                default: {}
            }
        } else {
            switch (event.event) {
                case ApplicationEventType::MENU_CHANGE: {
                    sendMenu();
                    break;
                }
                case ApplicationEventType::CHANGE_CHANNEL: {
                    players.push_back(std::async([](){ dataListener(current_channel().lock().get()); }));
                    break;
                }
                case ApplicationEventType::STOP: {
                    running = false;
                }
                default: {}
            }
        }
    }
}

MultiWriter &menu_output() {
    static auto *_menu_output = new MultiWriter();
    return *_menu_output;
}

void menuServer(uint16_t port) {
    TcpListener listener(port);
    bool running = true;
    std::list<std::future<void>> tasks_handles;
    std::shared_ptr<TcpStream> new_tcp_stream;

    while (running) {
        new_tcp_stream = listener.acceptClient();
        tasks_handles.push_back(std::async([&]() {
            UserConnection connection(menu_output(), new_tcp_stream);
            auto stream = connection.getStream();
            auto writer = connection.getWriter();
            {
                auto writer_lock = writer->lock();
                auto message = terminalScreen(menuToString(radio_menu().lock().get()));
                writer_lock->writeBytes(message);
                writer_lock->flushOutput();
            }
            try {
                handleUserInput(*stream);
            } catch (const IOException &e) {
                std::cerr << e.what() << std::endl;
            }
        }));

    }
}

MutexValue<std::unique_ptr<menu::OptionsListing>> &options_listing() {
    static auto *listing =
            new MutexValue<std::unique_ptr<menu::OptionsListing>>(std::unique_ptr<menu::OptionsListing>());
    return *listing;
}

std::shared_ptr<telnet::Stream> UserConnection::getStream() {
    return telnet_stream;
}

std::shared_ptr<MutexValue<ByteStreamWriter>> UserConnection::getWriter() {
    return writer;
}

std::string UserConnection::getIP() {
    return tcp_stream->ip();
}

std::string MenuEvent::toString() const {
    if (tag  == Tag::USER_EVENT) {
        return "KEY";
    } else {
        switch (event) {
            case ApplicationEventType::MENU_CHANGE:
                return "MENU";
            case ApplicationEventType::CHANGE_CHANNEL:
                return "CHANNEL";
            default:
                return "OTHER";
        }
    }
}
