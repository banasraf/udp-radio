/**
 * \author: Rafal Banas
 */
#include "menu.h"

void menu::Menu::up() {
    if (getActiveIndex() > 0) {
        --current_listing->active_index;
    }
}

void menu::Menu::down() {
    if (getActiveIndex() < current_listing->listing.size() - 1) {
        ++current_listing->active_index;
    }
}

void menu::Menu::enter() {
    current_listing->listing[getActiveIndex()].action();
    if (current_listing->listing[getActiveIndex()].menu_reference != nullptr) {
        current_listing = current_listing->listing[getActiveIndex()].menu_reference;
    }
}

std::vector<std::string> menu::Menu::getTextsListing() const {
    auto texts = std::vector<std::string>();
    for (auto &option: current_listing->listing) {
        texts.push_back(option.text);
    }
    return texts;
}

unsigned menu::Menu::getActiveIndex() const {
    return current_listing->active_index;
}

menu::Option &menu::OptionsListing::operator[](unsigned long index) {
    return listing[index];
}
