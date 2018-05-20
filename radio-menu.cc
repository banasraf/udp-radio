#include <list>
#include <memory>
#include "telnet-processor.h"
#include "radio-menu.h"



MutexValue<menu::Menu> &radio_menu() {
    static auto *options_listing = new menu::OptionsListing ({
            menu::Option("Stacja 1", [](){}),
            menu::Option("Stacja 2", [](){})
    });

    static auto *menu = new MutexValue<menu::Menu>(menu::Menu(options_listing));
    return *menu;
}

MutexValue<terminal::TextScreen> &text_screen() {
    static MutexValue<terminal::TextScreen> *_text_screen = new MutexValue(terminal::TextScreen(20));
    return *_text_screen;
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
    auto text_screen_lock = text_screen().lock();
    auto output_lock = menu_output().lock();
    MenuDrawer md(15);
    md.drawAt(text_screen_lock.get(), 3, 2, menu_lock.get());
    output_lock.writeBytes(text_screen_lock->renderToBytes());
    output_lock.flushOutput();
}

void eventLoop() {
    bool running = true;
    while (running) {
        auto event = event_stream().read();
        if (event.tag == MenuEvent::Tag::USER_EVENT) {
            switch (event.key) {
                case terminal::ActionKeyType::ARROW_DOWN: {
                    radio_menu().lock().get().down();
                    radio_menu().lock().get().enter();
                    sendMenu();
                    break;
                }
                case terminal::ActionKeyType::ARROW_UP: {
                    radio_menu().lock().get().up();
                    radio_menu().lock().get().enter();
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

    MenuDrawer menu_drawer(15);
    menu_drawer.drawAt(text_screen().lock().get(), 3, 1, radio_menu().lock().get());

    while (running) {
        new_tcp_stream = listener.acceptClient();
        tasks_handles.push_back(std::async([&]() {
            UserConnection connection(menu_output(), new_tcp_stream);
            auto stream = connection.getStream();
            auto writer = connection.getWriter();
            menu_output().addStream(writer);
            {
                auto writer_lock = writer->lock();
                writer_lock->writeBytes(text_screen().lock().get().initialBytes());
                writer_lock->writeBytes(text_screen().lock().get().renderToBytes());
                writer_lock->flushOutput();
            }
            handleUserInput(*stream);
            menu_output().removeStream(writer);
        }));

    }
}

std::shared_ptr<telnet::Stream> UserConnection::getStream() {
    return telnet_stream;
}

std::shared_ptr<MutexValue<ByteStreamWriter>> UserConnection::getWriter() {
    return writer;
}
