#ifndef TELNET_SERVER_RADIO_MENU_H
#define TELNET_SERVER_RADIO_MENU_H

#include "menu.h"
#include "terminal.h"
#include "threading.h"
#include "tcp-socket.h"
#include "application.h"
#include "text-screen.h"
#include "menu-drawer.h"
#include <iostream>

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


enum class ApplicationEventType {
    NEW_USER,
    MENU_CHANGE,
    STOP,
    NONE
};

struct MenuEvent {

    enum class Tag {
        APPLICATION_EVENT,
        USER_EVENT
    };

    Tag tag;
    union {
        terminal::ActionKeyType key;
        ApplicationEventType event;
    };

    explicit MenuEvent(terminal::ActionKeyType key): tag(Tag::USER_EVENT), key(key) {}

    explicit MenuEvent(ApplicationEventType _type)
            : tag(Tag::APPLICATION_EVENT),
              event(_type) {}

};

class UserConnection {

    InteractiveCmdHandler telnetCmdHandler;

    std::shared_ptr<TcpStream> tcp_stream;
    std::shared_ptr<telnet::Stream> telnet_stream;
    std::shared_ptr<MutexValue<ByteStreamWriter>> writer;

public:
    UserConnection(MultiWriter &output, const std::shared_ptr<TcpStream> &_tcp_stream):
            tcp_stream(_tcp_stream),
            telnet_stream(new telnet::Stream(*tcp_stream, telnetCmdHandler)),
            writer(new MutexValue(ByteStreamWriter(telnet_stream))) {
        output.addStream(writer);
    }

    std::shared_ptr<telnet::Stream> getStream();

    std::shared_ptr<MutexValue<ByteStreamWriter>> getWriter();

};

void handleUserInput(ByteStream &net_stream);

void eventLoop();

void menuServer(uint16_t port);

MutexValue<menu::Menu> &radio_menu();

MutexValue<terminal::TextScreen> &text_screen();

MultiWriter &menu_output();

MultiInputStream<MenuEvent> &event_stream();

#endif //TELNET_SERVER_RADIO_MENU_H
