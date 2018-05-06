/**
 * \author: Rafal Banas
 * Module provides tools for convenient rich text printing in terminal
 */

#ifndef TELNET_SERVER_TEXT_SCREEN_H
#define TELNET_SERVER_TEXT_SCREEN_H

#include "byte-stream.h"
#include "terminal.h"

#include <string>
#include <memory>

namespace terminal {

    /**
     * Namespace contains definitions of some terminal control sequences.
     */
    namespace control {

        const ByteStream::series_t &ClearScreenSeq();

        const ByteStream::series_t &HideCursorSeq();

        const ByteStream::series_t &ShowCursorSeq();

        /**
         * Interface for terminal control sequence.
         */
        class TerminalSequence {

        public:
            /**
             * Returns appropriate bytes sequence
             */
            virtual ByteStream::series_t toBytes() = 0;

        };

        /**
         * Moves cursor left by 'shift' columns
         */
        class MoveHorizontally: public TerminalSequence {

            int shift;

        public:
            ByteStream::series_t toBytes() override;

            explicit MoveHorizontally(int shift): shift(shift) {}

        };

        /**
         * Moves cursor down by 'shift' lines
         */
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

        /**
         * Regular text
         */
        class Text: public TerminalSequence {

            std::string text;

        public:
            ByteStream::series_t toBytes() override;

            explicit Text(const std::string &text): text(text) {}

        };

    }

    /**
     * Class representing terminal screen.
     */
    class TextScreen {

    public:
        using control_seq_t = std::shared_ptr<control::TerminalSequence>;
        using line_t = std::vector<control_seq_t>;
        using lines_sequence_t = std::vector<line_t>;

    private:
        lines_sequence_t lines_sequence;

    public:
        /**
         * Constructs text screen on given number of lines
         * @param lines_count
         */
        explicit TextScreen(unsigned long lines_count): lines_sequence(lines_count) {};

        /**
         * Clear given line
         */
        void clearLine(unsigned long line);

        /**
         * Clears whole reserved screen
         */
        void clearScreen();

        /**
         * Write text in given place
         */
        void writeAt(unsigned long line, int column, const std::string &text);

        /**
         * Write text in given place with reversed colors
         */
        void writeReversedStyleAt(unsigned long line, int column, const std::string &text);

        /**
         * Render to byte message that should be printed.
         * Clears all reserved lines and writes whole content in every bytes message.
         */
        ByteStream::series_t renderToBytes();

        /**
         * Bytes that should be printed before working on text screen.
         */
        ByteStream::series_t initialBytes();

    };

}

#endif //TELNET_SERVER_TEXT_SCREEN_H
