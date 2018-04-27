#ifndef TELNET_SERVER_MENU_H
#define TELNET_SERVER_MENU_H

#include <string>
#include <functional>
#include <cassert>

namespace menu {

    class OptionsListing;

    struct Option {

        std::string text;
        std::function<void()> action;
        OptionsListing *menu_reference;

        Option(const std::string &text, const std::function<void()> &action, OptionsListing *menu_reference = nullptr)
                : text(text),
                  action(action),
                  menu_reference(menu_reference) {}


    };

    struct OptionsListing {

        std::vector<Option> listing;
        unsigned active_index;

        explicit OptionsListing(const std::vector<Option> &options): listing(options), active_index(0) {
            assert(!listing.empty());
        }

        Option &operator [] (unsigned long index);

    };

    class Menu {

        OptionsListing* current_listing;

    public:
        void up();

        void down();

        void enter();

        std::vector<std::string> getTextsListing() const;

        unsigned getActiveIndex() const;

        explicit Menu(OptionsListing* current_listing): current_listing(current_listing) {}
    };
}

#endif //TELNET_SERVER_MENU_H
