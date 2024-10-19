#include <cstdint>
#ifndef MENU_H
#define MENU_H

#include "MenuItem.h"
#include "Watchy.h"

#define MENU_DIR_UP 1
#define MENU_DIR_DOWN 2

#define MENU_ITEMS_PER_PAGE 7//(200 - MENU_HEIGHT)/MENU_HEIGHT

class Menu{
    public:
        Menu(const Watchy *watchy){
            this->watchy = watchy;
            this->menuItems = new MenuItem*[MENU_LENGTH] {
                new MenuItem("About Watchy", ABOUT_APP),
                new MenuItem("Vibrate Motor", BUZZ_APP),
                new MenuItem("Show Accelerometer", ACCEL_APP),
                new MenuItem("Set Time", TIME_SET_APP),
                new MenuItem("Setup WiFi", WIFI_SETUP_APP),
                new MenuItem("Update Firmware", UPDATE_APP),
                new MenuItem("Sync NTP", NTP_APP),
            };
        }
        void renderMenu(bool partialRefresh);
        void moveCursor(uint8_t direction);
        MenuItem* getSelectedItem();
    private:
        MenuItem** menuItems;
        const Watchy *watchy = NULL;
};

#endif