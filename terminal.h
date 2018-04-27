#ifndef TELNET_SERVER_TERINAL_H
#define TELNET_SERVER_TERINAL_H

#include "byte-stream.h"

#include <optional>

namespace terminal {

    const uint8_t ESCAPE_BYTE = 27;
    const uint8_t CR_BYTE = 13;
    const uint8_t LF_BYTE = 10;

    bool isPrintableByte(uint8_t byte);

    enum class ActionKeyType {

        ENTER,
        ARROW_UP,
        ARROW_DOWN,
        ARROW_LEFT,
        ARROW_RIGHT,
        END_OF_STREAM  // special key for indicating byte stream's eof

    };

    struct Key {

        enum class Type {
          ACTION,
          PRINTABLE
        };

        Type type;
        union {
            ActionKeyType action;
            char printable;
        };

        Key(Type type, ActionKeyType action): type(type), action(action) {}

        Key(Type type, char printable): type(type), printable(printable) {}

        bool isAction() {
            return type == Type::ACTION;
        }

        bool isPrintable() {
            return type == Type::PRINTABLE;
        }

    };

    struct ActionKey: public Key {

        explicit ActionKey(ActionKeyType action): Key(Type::ACTION, action) {}

    };

    struct PrintableKey: public Key {

        explicit PrintableKey(char printable): Key(Type::PRINTABLE, printable) {}

    };

    class KeyStream {

        ByteStream &byte_stream;

        std::optional<Key> processCsiSequence();

        std::optional<Key> processEscapeSequence();

    public:
        Key nextKey();

        explicit KeyStream(ByteStream &byte_stream): byte_stream(byte_stream) {}

    };

}

#endif //TELNET_SERVER_TERINAL_H
