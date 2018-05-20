#include <iostream>
#include "application.h"
#include "text-screen.h"
#include "menu.h"
#include "menu-drawer.h"

/**
 * Telnet command handler. Sends WILL ECHO and DO LINEMODE on start.
 * Responds DON'T to any other WILL, WON'T to any other DO, WON'T to any DO and DON'T to any WON'T
 */
class InteractiveCmdHandler: public telnet::CommandHandler {

public:
    cmd_sequence_t init() override {
        return {
                telnet::Command::Will(telnet::OPT_ECHO),
                telnet::Command::Do(telnet::OPT_LINEMODE)
        };
    }

    cmd_sequence_t response(const telnet::Command &command) override {
        if (command.type == telnet::CMD_DONT && command.option != telnet::OPT_ECHO) {
            return { telnet::Command::Wont(command.option) };
        } else if (command.type == telnet::CMD_DO && command.option != telnet::OPT_ECHO) {
            return { telnet::Command::Wont(command.option) };
        } else if (command.type == telnet::CMD_WONT && command.option != telnet::OPT_LINEMODE) {
            return  { telnet::Command::Dont(command.option) };
        } else if (command.type == telnet::CMD_WILL && command.option != telnet::OPT_LINEMODE) {
            return { telnet::Command::Dont(command.option) };
        }
        return cmd_sequence_t();
    }

};

void init_terminal(terminal::TextScreen &text_screen, telnet::Stream &stream) {
    stream.writeBytes(terminal::control::HideCursorSeq());
    stream.writeBytes(text_screen.initialBytes());
}

void actionA(terminal::TextScreen &text_screen) {
    text_screen.clearLine(4);
    text_screen.writeAt(4, 1, "A");
}

void actionB1(terminal::TextScreen &text_screen) {
    text_screen.clearLine(4);
    text_screen.writeAt(4, 1, "B1");
}

void actionB2(terminal::TextScreen &text_screen) {
    text_screen.clearLine(4);
    text_screen.writeAt(4, 1, "B2");
}

/**
 * Main application loop for handling keyboard events.
 */
void event_loop(terminal::TextScreen &text_screen,
                menu::Menu &menu,
                telnet::Stream &stream,
                bool &client_exit) {
    auto key_stream = terminal::KeyStream(stream);
    MenuDrawer menu_drawer(text_screen, 3);

    menu_drawer.drawAt(1, 0, menu);
    stream.writeBytes(text_screen.renderToBytes());
    stream.flushOutput();

    bool client_disconnect = false;

    while (!client_exit && !client_disconnect) {
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
                    client_disconnect = true;
                    break;
                }
                default: {
                }
            }
            if (!client_exit && !client_disconnect) {
                menu_drawer.drawAt(1, 0, menu);
                stream.writeBytes(text_screen.renderToBytes());
                stream.flushOutput();
            } else if (client_exit) {
                stream.writeBytes(text_screen.renderToBytes());
                stream.writeBytes(terminal::control::ShowCursorSeq());
                stream.flushOutput();
                std::cout << "Client has exited." << std::endl;
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
                    menu::Option("OpcjaA", [&]() { actionA(text_screen); }),
                    menu::Option("OpcjaB", []() {}),
                    menu::Option("Koniec", [&]() { client_exit = true; text_screen.clearScreen(); })
            });

    auto options_listing2 = menu::OptionsListing(
            {
                    menu::Option("OpcjaB1", [&]() { actionB1(text_screen); }),
                    menu::Option("OpcjaB2", [&]() { actionB2(text_screen); }),
                    menu::Option("Wstecz", [](){}, &options_listing1)
            });

    options_listing1[1].menu_reference = &options_listing2;

    menu::Menu menu(&options_listing1);
    event_loop(text_screen, menu, stream, client_exit);
}