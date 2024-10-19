#ifndef APP_WIFISETUP_H
#define APP_WIFISETUP_H
#include "../WatchyApp.h"

class WifiSetupApp : public WatchyApp {
public:
    WifiSetupApp(Watchy *watchy) : WatchyApp(watchy) {}
    void init() override;
    void destroy() override;
    void handleButtonPress(WatchyButton button) override;
    static void _configModeCallback(WiFiManager *myWiFiManager);
};

#endif