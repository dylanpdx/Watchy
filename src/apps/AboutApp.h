#ifndef APP_ABOUT_H
#define APP_ABOUT_H
#include "../WatchyApp.h"

class AboutApp : public WatchyApp {
public:
    AboutApp(Watchy *watchy) : WatchyApp(watchy) {}
    void init() override;
    void render();
    void destroy() override;
    void handleButtonPress(WatchyButton button) override;
};

#endif