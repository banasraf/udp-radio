#ifndef TELNET_SERVER_TEXT_SCREEN_H
#define TELNET_SERVER_TEXT_SCREEN_H

#include "byte-stream.h"
#include "terminal.h"

#include <string>
#include <memory>

namespace terminal {

    namespace control {

        const ByteStream::series_t &ClearScreenSeq();

        const ByteStream::series_t &HideCursorSeq();

        class TerminalSequence {

        public:
            virtual ByteStream::series_t toBytes() = 0;

        };

        class MoveHorizontally: public TerminalSequence {

            int shift;

        public:
            ByteStream::series_t toBytes() override;

            explicit MoveHorizontally(int shift): shift(shift) {}

        };

        class MoveVertically: public TerminalSequence {

            int shift;

        public:
            ByteStream::series_t toBytes() override;

            explicit MoveVertically(int shift): shift(shift) {}

        };

        class ClearLine: public TerminalSequence {

        public:
            ByteStream::series_t toBytes() override;

        };

        class SaveCursorPosition: public TerminalSequence {

        public:
            ByteStream::series_t toBytes() override;

        };

        class RestoreCursorPosition: public TerminalSequence {

        public:
            ByteStream::series_t toBytes() override;

        };

        class SetStyle: public TerminalSequence {

            int style;

        public:
            static const int REVERSED = 7;
            static const int NORMAL = 0;


            ByteStream::series_t toBytes() override;

            explicit SetStyle(int style): style(style) {}

        };


        class Text: public TerminalSequence {

            std::string text;

        public:
            ByteStream::series_t toBytes() override;

            explicit Text(const std::string &text): text(text) {}

        };

    }

    class TextScreen {

    public:
        using control_seq_t = std::shared_ptr<control::TerminalSequence>;
        using line_t = std::vector<control_seq_t>;
        using line_sequence_t = std::vector<line_t>;

    private:
        line_sequence_t line_sequence;

    public:
        explicit TextScreen(unsigned long lines_count): line_sequence(lines_count) {};

        void clearLine(unsigned long line);

        void clearScreen();

        void writeAt(unsigned long line, int column, const std::string &text);

        void writeReversedStyleAt(unsigned long line, int column, const std::string &text);

        ByteStream::series_t renderToBytes();

        ByteStream::series_t initialBytes();

    };

}

#endif //TELNET_SERVER_TEXT_SCREEN_H
