#include <cstdint>
#ifndef MENU_H
#define MENU_H

#include "MenuItem.h"
#include "Watchy.h"

#define MENU_DIR_UP 1
#define MENU_DIR_DOWN 2

class Menu{
    public:
        Menu(const Watchy *watchy){
            this->cursor = 0;
            this->watchy = watchy;
            this->menuItems = new MenuItem*[MENU_LENGTH] {
                new MenuItem("About Watchy", &Watchy::showAbout),
                new MenuItem("Vibrate Motor", BUZZ_APP),
                new MenuItem("Show Accelerometer", &Watchy::showAccelerometer),
                new MenuItem("Set Time", &Watchy::setTime),
                new MenuItem("Setup WiFi", &Watchy::setupWifi),
                new MenuItem("Update Firmware", &Watchy::showUpdateFW),
                new MenuItem("Sync NTP", &Watchy::showSyncNTP),
            };
        }
        void renderMenu(bool partialRefresh);
        void moveCursor(uint8_t direction);
        MenuItem* getSelectedItem();
    private:
        uint8_t cursor;
        MenuItem** menuItems;
        const Watchy *watchy = NULL;
};

#endif