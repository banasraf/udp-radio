/**
 * \author: Rafal Banas
 */

#include "text-screen.h"

using namespace std;

ByteStream::series_t terminal::control::MoveHorizontally::toBytes() {
    if (shift == 0) {
        return ByteStream::series_t();
    } else {
        auto result = ByteStream::series_t();
        result.push_back(ESCAPE_BYTE);
        result.push_back('[');
        u_char final_byte;
        string arg;
        if (shift > 0) {
            final_byte = 'C';
            arg = to_string(shift);
        } else {
            final_byte = 'D';
            arg = to_string(-shift);
        }
        result.insert(result.end(), arg.begin(), arg.end());
        result.push_back(final_byte);
        return result;
    }
}

ByteStream::series_t terminal::control::MoveVertically::toBytes() {
    if (shift == 0) {
        return ByteStream::series_t();
    } else {
        auto result = ByteStream::series_t();
        result.push_back(ESCAPE_BYTE);
        result.push_back('[');
        u_char final_byte;
        string arg;
        if (shift > 0) {
            final_byte = 'B';
            arg = to_string(shift);
        } else {
            final_byte = 'A';
            arg = to_string(-shift);
        }
        result.insert(result.end(), arg.begin(), arg.end());
        result.push_back(final_byte);
        return result;
    }
}


ByteStream::series_t terminal::control::ClearLine::toBytes() {
    return ByteStream::series_t({ESCAPE_BYTE, '[', '2', 'K'});
}

ByteStream::series_t terminal::control::Text::toBytes() {
    return ByteStream::series_t(text.begin(), text.end());
}

void terminal::TextScreen::clearLine(unsigned long line) {
    auto seq = new control::ClearLine();
    lines_sequence[line].clear();
    lines_sequence[line].push_back(shared_ptr<control::TerminalSequence>(seq));
}

void terminal::TextScreen::clearScreen() {
    for (unsigned long i = 0; i < lines_sequence.size(); ++i) {
        clearLine(i);
    }
}

void terminal::TextScreen::writeAt(unsigned long line, int column, const std::string &text) {
    auto save_position = new control::SaveCursorPosition();
    auto move_to = new control::MoveHorizontally(column);
    auto write_text = new control::Text(text);
    auto restore_position = new control::RestoreCursorPosition();
    auto to_append = line_t(
            {
                           control_seq_t(save_position),
                           control_seq_t(move_to),
                           control_seq_t(write_text),
                           control_seq_t(restore_position)
            });
    lines_sequence[line].insert(lines_sequence[line].end(), to_append.begin(), to_append.end());
}

ByteStream::series_t terminal::TextScreen::renderToBytes() {
    auto bytes = ByteStream::series_t();
    bytes.insert(bytes.end(), control::ClearScreenSeq().begin(), control::ClearScreenSeq().end());
    int v_shift = 0;
    auto next_line_bytes = control::MoveVertically(1).toBytes();
    for (auto &line: lines_sequence) {
        for (auto &control_seq : line) {
            auto control_bytes = control_seq->toBytes();
            bytes.insert(bytes.end(), control_bytes.begin(), control_bytes.end());
        }
        bytes.insert(bytes.end(), next_line_bytes.begin(), next_line_bytes.end());
        ++v_shift;
    }
    auto move_back_bytes = control::MoveVertically(-v_shift).toBytes();
    bytes.insert(bytes.end(), move_back_bytes.begin(), move_back_bytes.end());
    return bytes;
}

ByteStream::series_t terminal::TextScreen::initialBytes() {
    auto bytes = ByteStream::series_t();
    int i;
    for (i = 0; i < (int) lines_sequence.size(); ++i) {
        bytes.push_back(CR_BYTE);
        bytes.push_back(LF_BYTE);
    }
    auto move_back_bytes = control::MoveVertically(-i).toBytes();
    bytes.insert(bytes.end(), move_back_bytes.begin(), move_back_bytes.end());
    return bytes;
}

void terminal::TextScreen::writeReversedStyleAt(unsigned long line, int column, const std::string &text) {
    auto reverse = new control::SetStyle(control::SetStyle::REVERSED);
    auto normal = new control::SetStyle(control::SetStyle::NORMAL);
    lines_sequence[line].push_back(control_seq_t(reverse));
    writeAt(line, column, text);
    lines_sequence[line].push_back(control_seq_t(normal));
}

ByteStream::series_t terminal::control::SaveCursorPosition::toBytes() {
    return ByteStream::series_t({ESCAPE_BYTE, '[', 's'});
}

ByteStream::series_t terminal::control::RestoreCursorPosition::toBytes() {
    return ByteStream::series_t({ESCAPE_BYTE, '[', 'u'});
}

ByteStream::series_t terminal::control::SetStyle::toBytes() {
    string style_str = to_string(style);
    ByteStream::series_t bytes;
    bytes.push_back(ESCAPE_BYTE);
    bytes.push_back('[');
    bytes.insert(bytes.end(), style_str.begin(), style_str.end());
    bytes.push_back('m');
    return bytes;
}

const ByteStream::series_t &terminal::control::ClearScreenSeq() {
    static ByteStream::series_t *clear_screen_seq = new ByteStream::series_t({ESCAPE_BYTE, '[', '2', 'J'});
    return *clear_screen_seq;
}

const ByteStream::series_t &terminal::control::HideCursorSeq() {
    static ByteStream::series_t *hide_cursor_seq = new ByteStream::series_t({ESCAPE_BYTE, '[', '?', '2', '5', 'l'});
    return *hide_cursor_seq;
}

const ByteStream::series_t &terminal::control::ShowCursorSeq() {
    static ByteStream::series_t *show_cursor_seq = new ByteStream::series_t({ESCAPE_BYTE, '[', '?', '2', '5', 'h'});
    return *show_cursor_seq;
}
