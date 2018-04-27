#include "menu-drawer.h"

void MenuDrawer::drawAt(unsigned long line, int column, const menu::Menu &menu) {
    unsigned long active_index = menu.getActiveIndex();
    auto texts = menu.getTextsListing();

    for (unsigned long i = 0; i < lines_count; i++) {
        text_screen.clearLine(line + i);
        if (i == active_index) {
            text_screen.writeReversedStyleAt(line + i, column, texts[i]);
        } else if (i < texts.size()) {
            text_screen.writeAt(line + i, column, texts[i]);
        }
    }
}
