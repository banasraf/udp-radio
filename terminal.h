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

    /**
     * Union type for keyboard key.
     */
    struct Key {

        enum class Type {
          ACTION, /// type for special keyboard keys
          PRINTABLE /// type for non-action keys
        };

        /**
         * Type of key
         */
        Type type;

        union {
            ActionKeyType action;
            char printable;
        };

        /**
         * Constructor for action key
         */
        Key(Type type, ActionKeyType action): type(type), action(action) {}


        /**
         * Constructor for printable key
         * @param type
         * @param printable
         */
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

    /**
     * Class for mapping byte stream to keyboard events
     */
    class KeyStream {

        ByteStream &byte_stream;

        std::optional<Key> processCsiSequence();

        std::optional<Key> processEscapeSequence();

    public:
        /**
         * Get next key from stream
         */
        Key nextKey();

        /**
         * Constructs key stream as mapper from given byte stream
         * @param byte_stream
         */
        explicit KeyStream(ByteStream &byte_stream): byte_stream(byte_stream) {}

    };

}

#endif //TELNET_SERVER_TERINAL_H
