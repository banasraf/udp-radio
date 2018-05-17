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

void serveUser(ByteStream &net_stream) {
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

void sendMenu(MultiWriter &output) {
    auto menu_lock = radio_menu().lock();
    auto text_screen_lock = text_screen().lock();
    auto output_lock = output.lock();
    MenuDrawer md(15);
    md.drawAt(text_screen_lock.get(), 3, 2, menu_lock.get());
    output_lock.writeBytes(text_screen_lock->renderToBytes());
    output_lock.flushOutput();
}

void eventLoop(MultiWriter &output) {
    bool running = true;
    while (running) {
        auto event = event_stream().read();
        if (event.tag == MenuEvent::Tag::USER_EVENT) {
            switch (event.key) {
                case terminal::ActionKeyType::ARROW_DOWN: {
                    radio_menu().lock().get().down();
                    radio_menu().lock().get().enter();
                    sendMenu(output);
                    break;
                }
                case terminal::ActionKeyType::ARROW_UP: {
                    radio_menu().lock().get().up();
                    radio_menu().lock().get().enter();
                    sendMenu(output);
                    break;
                }
                default: {}
            }
        } else {
            switch (event.event) {
                case ApplicationEventType::MENU_CHANGE: {
                    sendMenu(output);
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

std::shared_ptr<telnet::Stream> UserConnection::getStream() {
    return telnet_stream;
}

std::shared_ptr<MutexValue<ByteStreamWriter>> UserConnection::getWriter() {
    return writer;
}
