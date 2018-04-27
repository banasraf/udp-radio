#include <iostream>
#include "application.h"
#include "text-screen.h"
#include "menu.h"
#include "menu-drawer.h"

void init_terminal(terminal::TextScreen &text_screen, telnet::Stream &stream) {
    stream.writeBytes(text_screen.initialBytes());
    stream.flushOutput();
}

void actionA(terminal::TextScreen &text_screen) {
    text_screen.clearLine(5);
    text_screen.writeAt(4, 1, "A");
}

void actionB1(terminal::TextScreen &text_screen) {
    text_screen.clearLine(5);
    text_screen.writeAt(4, 1, "B1");
}

void actionB2(terminal::TextScreen &text_screen) {
    text_screen.clearLine(5);
    text_screen.writeAt(4, 1, "B2");
}

void event_loop(terminal::TextScreen &text_screen,
                menu::Menu &menu,
                telnet::Stream &stream,
                bool &client_exit) {
    auto key_stream = terminal::KeyStream(stream);
    MenuDrawer menu_drawer(text_screen, 3);

    menu_drawer.drawAt(1, 0, menu);
    stream.writeBytes(text_screen.renderToBytes());
    stream.flushOutput();

    while (!client_exit) {
        auto key = key_stream.nextKey();
        if (key.isAction()) {
            switch (key.action) {
                case terminal::ActionKeyType::ARROW_UP: {
                    menu.up();
                    break;
                }
                case terminal::ActionKeyType::ARROW_DOWN: {
                    menu.down();
                    break;
                }
                case terminal::ActionKeyType::ENTER: {
                    menu.enter();
                    break;
                }
                case terminal::ActionKeyType::END_OF_STREAM: {
                    client_exit = true;
                    break;
                }
                default: {
                }
            }
            if (!client_exit) {
                menu_drawer.drawAt(1, 0, menu);
                stream.writeBytes(text_screen.renderToBytes());
                stream.flushOutput();
            } else {
                std::cout << "Client has disconnected." << std::endl;
            }
        }
    }
}

void serve_client(TcpStream &tcp_stream) {

    auto telnetCmdHandler = InteractiveCmdHandler();
    auto stream = telnet::Stream(tcp_stream, telnetCmdHandler);
    auto text_screen = terminal::TextScreen(SCREEN_SIZE);
    bool client_exit = false;

    init_terminal(text_screen, stream);

    auto options_listing1 = menu::OptionsListing(
            {
                    menu::Option("OpcjaA", [&]() {actionA(text_screen);}),
                    menu::Option("OpcjaB", []() {}),
                    menu::Option("Koniec", [&]() { client_exit = true; })
            });

    auto options_listing2 = menu::OptionsListing(
            {
                    menu::Option("OpcjaB1", [&]() {actionB1(text_screen);}),
                    menu::Option("OpcjaB2", [&]() {actionB2(text_screen);}),
                    menu::Option("Wstecz", [](){}, &options_listing1)
            });

    options_listing1[1].menu_reference = &options_listing2;

    menu::Menu menu(&options_listing1);
    event_loop(text_screen, menu, stream, client_exit);
}