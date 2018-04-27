/**
 * \author: Rafal Banas
 * Module provides basic telnet state machine and defines some of the protocol's constants.
 */

#ifndef TELNET_SERVER_TELNET_PROCESSOR_H
#define TELNET_SERVER_TELNET_PROCESSOR_H

#include "byte-stream.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace telnet {

    const uint8_t CMD_WILL = 251;
    const uint8_t CMD_WONT = 252;
    const uint8_t CMD_DO = 253;
    const uint8_t CMD_DONT = 254;
    const uint8_t CMD_NOP = 241;
    const uint8_t CMD_SE = 240;
    const uint8_t CMD_SB = 250;
    const uint8_t CMD_IAC = 255;
    const uint8_t CMD_UNKNOWN = 0;


    const uint8_t OPT_ECHO = 1;
    const uint8_t OPT_LINEMODE = 34;
    const uint8_t OPT_UNKNOWN = 254;


    struct Command {

        using sub_options_t = std::vector<uint8_t>;

        uint8_t type;
        uint8_t option;
        sub_options_t sub_options;

        Command(uint8_t type, uint8_t option, sub_options_t sub_options)
                : type(type),
                  option(option),
                  sub_options(std::move(sub_options)) {};

        ByteStream::series_t toBytes() const;

        static Command Will(uint8_t option);

        static Command Wont(uint8_t option);

        static Command Do(uint8_t option);

        static Command Dont(uint8_t option);

        static Command Empty();

    private:
        ByteStream::series_t sbToBytes() const ;

    };

    class CommandHandler {

    public:

        using cmd_sequence_t = std::vector<Command>;

        /**
         * Returns response to a command
         */
        virtual cmd_sequence_t response(const Command &command) = 0;

        /**
         * Returns commands to send on connection initiation
         */
        virtual std::vector<Command> init() = 0;

    };

    class Stream: public ByteStream {

        struct EscapedByte {

            enum class Tag {CMD_BYTE, DATA_BYTE} tag;
            uint8_t value;

            EscapedByte(Tag tag, uint8_t value): tag(tag), value(value) {}

        };

        ByteStream &net_stream;
        CommandHandler &cmd_handler;

        EscapedByte readEscaped();

        Command process_will();

        Command process_wont();

        Command process_do();

        Command process_dont();

        Command process_sb();

        // Process whole command and return following byte
        uint8_t goThroughCommand(uint8_t cmd_type);

    public:

        explicit Stream(ByteStream &net_stream, CommandHandler &cmd_handler)
                : net_stream(net_stream),
                  cmd_handler(cmd_handler) {
            sendCommands(cmd_handler.init());
        }

        uint8_t readByte() override;

        series_t readBytes(size_type count) override;

        void writeByte(uint8_t byte) override;

        void writeBytes(const series_t &series) override;

        void flushOutput() override;

        void sendCommands(const CommandHandler::cmd_sequence_t &commands);

    };

}

#endif //TELNET_SERVER_TELNET_PROCESSOR_H
