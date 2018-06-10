/**
 * \author: Rafal Banas
 * Module with menu model
 */
#ifndef TELNET_SERVER_MENU_H
#define TELNET_SERVER_MENU_H

#include <string>
#include <functional>
#include <cassert>

namespace menu {


    class OptionsListing;

    /**
     * Single menu's option
     */
    struct Option {

        std::string text;  /// option text
        std::function<void()> action; /// action callback
        OptionsListing *menu_reference; /// option's sub-menu

        Option(const std::string &text, const std::function<void()> &action, OptionsListing *menu_reference = nullptr)
                : text(text),
                  action(action),
                  menu_reference(menu_reference) {}


    };

    /**
     * Options list
     */
    struct OptionsListing {

        std::vector<Option> listing;
        unsigned active_index; /// current option

        explicit OptionsListing(const std::vector<Option> &options): listing(options), active_index(0) {}

        /**
         * Select option at given index
         */
        Option &operator [] (unsigned long index);

    };

    /**
     * Vertical menu model
     */
    class Menu {

        OptionsListing* current_listing;

    public:
        /**
         * Select previous option. Does nothing when first option is selected
         */
        void up();

        /**
         * Select next option. Does nothing when last option is selected
         */
        void down();

        /**
         * Enter the option, change current listing and execute callback
         */
        void enter();

        /**
         * Get texts from current listing
         */
        std::vector<std::string> getTextsListing() const;

        /**
         * Returns index of currently selected option
         */
        unsigned getActiveIndex() const;

        explicit Menu(OptionsListing* current_listing): current_listing(current_listing) {}

        void setListing(OptionsListing *listing);

        bool empty() const;

    };
}

#endif //TELNET_SERVER_MENU_H
