/**
 * \author: Rafal Banas
 * Helper module for rendering Menus on TextScreen
 */
#ifndef TELNET_SERVER_MENU_DRAWER_H
#define TELNET_SERVER_MENU_DRAWER_H

#include "menu.h"
#include "text-screen.h"

class MenuDrawer {

    terminal::TextScreen &text_screen;  /// text screen which menu will be drawn on
    unsigned long lines_count;  /// number of lines used for drawing menu

public:
    explicit MenuDrawer(terminal::TextScreen &text_screen, unsigned long lines_count)
            : text_screen(text_screen),
              lines_count(lines_count) {};

    /**
     * Draw menu at given position
     */
    void drawAt(unsigned long line, int column, const menu::Menu &menu);

};

#endif //TELNET_SERVER_MENU_DRAWER_H
