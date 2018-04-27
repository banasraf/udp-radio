#include "terminal.h"

terminal::Key terminal::KeyStream::nextKey() {
    uint8_t byte;
    try {
        byte = byte_stream.readByte();
    } catch (const Eof &eof_exception) {
        return ActionKey(ActionKeyType::END_OF_STREAM);
    }

    if (isPrintableByte(byte)) {
        return PrintableKey(byte);
    } else if (byte == ESCAPE_BYTE) {
        auto processed = processEscapeSequence();
        if (processed) {
            return *processed;
        } else {
            return nextKey();
        }
    } else if (byte == CR_BYTE) {
        return ActionKey(ActionKeyType::ENTER);
    } else {
        return nextKey();
    }
}

std::optional<terminal::Key> terminal::KeyStream::processCsiSequence() {
    uint8_t byte;
    try {
        byte = byte_stream.readByte();
    } catch (const Eof &eof_exception) {
        return std::make_optional(ActionKey(ActionKeyType::END_OF_STREAM));
    }
    switch (byte) {
        case 'A': {
            return std::make_optional(ActionKey(ActionKeyType::ARROW_UP));
        }
        case 'B': {
            return std::make_optional(ActionKey(ActionKeyType::ARROW_DOWN));
        }
        case 'C': {
            return std::make_optional(ActionKey(ActionKeyType::ARROW_RIGHT));
        }
        case 'D': {
            return std::make_optional(ActionKey(ActionKeyType::ARROW_LEFT));
        }
        default: {
            return {};
        }

    }
}

std::optional<terminal::Key> terminal::KeyStream::processEscapeSequence() {
    uint8_t byte;
    try {
        byte = byte_stream.readByte();
    } catch (const Eof &eof_exception) {
        return std::make_optional(ActionKey(ActionKeyType::END_OF_STREAM));
    }
    if (byte == '[') {
        return processCsiSequence();
    } else {
        return std::make_optional(nextKey());
    }
}

bool terminal::isPrintableByte(uint8_t byte) {
    return byte >= 32 && byte <= 126;
}
