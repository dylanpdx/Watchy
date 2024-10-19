#ifndef APP_ACCEL_H
#define APP_ACCEL_H
#include "../WatchyApp.h"

class AccelApp : public WatchyApp {
public:
    AccelApp(Watchy *watchy) : WatchyApp(watchy) {}
    void init() override;
    void destroy() override;
    void handleButtonPress(WatchyButton button) override;
private:
    bool running = true;
};

#endif