#ifndef APP_UPDATE_H
#define APP_UPDATE_H
#include "../WatchyApp.h"

class UpdateApp : public WatchyApp {
public:
    UpdateApp(Watchy *watchy) : WatchyApp(watchy) {}
    void init() override;
    void destroy() override;
    void handleButtonPress(WatchyButton button) override;
    void updateFWBegin();
private:
    bool updating = false;
};

#endif