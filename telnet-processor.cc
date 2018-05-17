/**
 * \author: Rafal Banas
 */

#include "telnet-processor.h"

#include <iostream>
using namespace std;

uint8_t telnet::Stream::readByte() {
    auto escaped = readEscaped();
    switch (escaped.tag) {
        case EscapedByte::Tag::CMD_BYTE: {
            return goThroughCommand(escaped.value);
        }
        default: {
            return escaped.value;
        }
    }
}

uint8_t telnet::Stream::goThroughCommand(uint8_t cmd_type) {
    CommandHandler::cmd_sequence_t response;
    switch (cmd_type) {
        case CMD_WILL: {
            response = cmd_handler.response(process_will());
            break;
        }
        case CMD_WONT: {
            response = cmd_handler.response(process_wont());
            break;
        }
        case CMD_DO: {
            response = cmd_handler.response(process_do());
            break;
        }
        case CMD_DONT: {
            response = cmd_handler.response(process_dont());
            break;
        }
        case CMD_SB: {
            response = cmd_handler.response(process_sb());
            break;
        }
        default: {}
    }
    sendCommands(response);
    return readByte();
}

telnet::Command telnet::Stream::process_will() {
    return Command::Will(readEscaped().value);
}

telnet::Command telnet::Stream::process_wont() {
    return Command::Wont(readEscaped().value);
}

telnet::Command telnet::Stream::process_do() {
    return Command::Do(readEscaped().value);
}

telnet::Command telnet::Stream::process_dont() {
    return Command::Dont(readEscaped().value);
}

telnet::Command telnet::Stream::process_sb() {
    auto  sub_options = Command::sub_options_t();
    auto option = readEscaped().value;
    auto byte = readEscaped();
    while (byte.tag != EscapedByte::Tag::CMD_BYTE || byte.value != CMD_SE) {
        sub_options.push_back(byte.value);
        byte = readEscaped();
    }
    return Command(CMD_SE, option, sub_options);
}

telnet::Stream::EscapedByte telnet::Stream::readEscaped() {
    auto byte = net_stream.readByte();
    if (byte == CMD_IAC) {
        auto value = net_stream.readByte();
        if (value == CMD_IAC) {
            return EscapedByte(EscapedByte::Tag::DATA_BYTE, CMD_IAC);
        } else {
            return EscapedByte(EscapedByte::Tag::CMD_BYTE, value);
        }
    } else {
        return EscapedByte(EscapedByte::Tag::DATA_BYTE, byte);
    }
}

void telnet::Stream::sendCommands(const telnet::CommandHandler::cmd_sequence_t &commands) {
    for (auto &command: commands) {
        net_stream.writeBytes(command.toBytes());
    }
    net_stream.flushOutput();
}

ByteStream::series_t telnet::Stream::readBytes(ByteStream::size_type count) {
    auto result = series_t();
    for (size_type i = 0; i < count; ++i) {
        try {
            auto byte = readByte();
            result.push_back(byte);
        } catch (const Eof &eof_exception) {
            if (i > 0) {
                return result;
            } else {
                throw;
            }
        }
    }
    return result;
}

void telnet::Stream::writeByte(uint8_t byte) {
    if (byte == CMD_IAC) {
        net_stream.writeBytes({CMD_IAC, CMD_IAC});
    } else {
        net_stream.writeByte(byte);
    }
}

void telnet::Stream::writeBytes(const ByteStream::series_t &series) {
    for (auto byte: series) {
        writeByte(byte);
    }
}

void telnet::Stream::flushOutput() {
    net_stream.flushOutput();
}


telnet::Command telnet::Command::Dont(uint8_t option) {
    return telnet::Command(CMD_DONT, option, sub_options_t());
}

telnet::Command telnet::Command::Empty() {
    return telnet::Command(CMD_NOP, OPT_UNKNOWN, sub_options_t());
}

telnet::Command telnet::Command::Will(uint8_t option) {
    return telnet::Command(CMD_WILL, option, sub_options_t());
}

telnet::Command telnet::Command::Wont(uint8_t option) {
    return telnet::Command(CMD_WONT, option, sub_options_t());
}

telnet::Command telnet::Command::Do(uint8_t option) {
    return telnet::Command(CMD_DO, option, sub_options_t());
}

ByteStream::series_t telnet::Command::toBytes() const {
    if (type == CMD_SB) {
        return sbToBytes();
    } else {
        return {CMD_IAC, type, option};
    }
}

ByteStream::series_t telnet::Command::sbToBytes() const {
    auto bytes = ByteStream::series_t();
    bytes.push_back(CMD_IAC);
    bytes.push_back(CMD_SB);
    bytes.push_back(option);
    for (auto byte: sub_options) {
        bytes.push_back(byte);
    }
    bytes.push_back(CMD_IAC);
    bytes.push_back(CMD_SE);
    return bytes;
}
