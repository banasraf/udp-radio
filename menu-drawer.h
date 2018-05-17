/**
 * \author: Rafal Banas
 * Helper module for rendering Menus on TextScreen
 */
#ifndef TELNET_SERVER_MENU_DRAWER_H
#define TELNET_SERVER_MENU_DRAWER_H

#include "menu.h"
#include "text-screen.h"

class MenuDrawer {

    unsigned long lines_count;  /// number of lines used for drawing menu

public:
    explicit MenuDrawer(unsigned long lines_count): lines_count(lines_count) {};

    /**
     * Draw menu at given position
     */
    void drawAt(terminal::TextScreen &text_screen, unsigned long line, int column, const menu::Menu &menu);

};

#endif //TELNET_SERVER_MENU_DRAWER_H
