/**
 * \author: Rafal Banas
 * Helper module for rendering Menus on TextScreen
 */
#ifndef TELNET_SERVER_MENU_DRAWER_H
#define TELNET_SERVER_MENU_DRAWER_H

#include "menu.h"
#include "text-screen.h"

std::string menuToString(const menu::Menu &menu);

ByteStream::series_t terminalScreen(const std::string &text);

#endif //TELNET_SERVER_MENU_DRAWER_H
