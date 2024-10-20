#ifndef APP_BUZZ_H
#define APP_BUZZ_H
#include "../WatchyApp.h"

class BuzzApp : public WatchyApp {
public:
    BuzzApp(Watchy *watchy) : WatchyApp(watchy) {}
    void init() override;
    void render();
    void destroy() override;
    void handleButtonPress(WatchyButton button) override;
};

#endif