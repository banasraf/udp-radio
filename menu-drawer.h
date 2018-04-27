#ifndef TELNET_SERVER_MENU_DRAWER_H
#define TELNET_SERVER_MENU_DRAWER_H

#include "menu.h"
#include "text-screen.h"

class MenuDrawer {

    terminal::TextScreen &text_screen;
    unsigned long lines_count;

public:
    explicit MenuDrawer(terminal::TextScreen &text_screen, unsigned long lines_count)
            : text_screen(text_screen),
              lines_count(lines_count) {};

    void drawAt(unsigned long line, int column, const menu::Menu &menu);

};

#endif //TELNET_SERVER_MENU_DRAWER_H
