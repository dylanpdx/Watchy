#ifndef MENUITEM_H
#define MENUITEM_H

#include <cstring>
#include "Watchy.h"
#include "WatchyApp.h"

class MenuItem {
    private:
        char* name;
        void (Watchy::*handler)()=NULL;
        uint8_t app=0;

    public:
        MenuItem(const char* name, void (Watchy::*handler)()) : handler(handler) {
            this->name = new char[strlen(name) + 1];
            strcpy(this->name, name);
        }

        MenuItem(const char* name, uint8_t app) : app(app) {
            this->name = new char[strlen(name) + 1];
            strcpy(this->name, name);
        }

        ~MenuItem() {
            delete[] name;
        }

        const char* getName() const {
            return name;
        }

        void executeHandler(Watchy &watchyInstance) const {
            if (handler!=NULL) {
                (watchyInstance.*handler)();
            }else if (app!=0) {
                watchyInstance.launchApp(app);
            }
        }
};

#endif