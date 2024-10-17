#ifndef MENUITEM_H
#define MENUITEM_H

#include <cstring>
#include "Watchy.h"

class MenuItem {
    private:
        char* name;
        void (Watchy::*handler)();

    public:
        MenuItem(const char* name, void (Watchy::*handler)()) : handler(handler) {
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
            if (handler) {
                (watchyInstance.*handler)();
            }
        }
};

#endif