#include <sstream>
#include "menu-drawer.h"

std::string menuToString(const menu::Menu &menu) {
    std::stringstream ss;
    ss << "------------------------------------------------------------------------\n\r";
    ss << "  SIK Radio\n\r";
    ss << "------------------------------------------------------------------------\n\r";

    if (!menu.empty()) {
        auto texts = menu.getTextsListing();
        unsigned long active_index = menu.getActiveIndex();
        for (unsigned long i = 0; i < texts.size(); i++) {
            if (i == active_index) {
                ss << "  > " << texts[i] << "\n\r";
            } else if (i < texts.size()) {
                ss << "    " << texts[i] << "\n\r";
            }
        }
    }
    ss << "------------------------------------------------------------------------\n\r";
    return ss.str();
}

ByteStream::series_t terminalScreen(const std::string &text) {
    ByteStream::series_t result;
    const ByteStream::series_t &clear_screen = terminal::control::ClearScreenSeq();
    result.insert(result.end(), clear_screen.begin(), clear_screen.end());
    result.insert(result.end(), text.begin(), text.end());
    return result;
}
