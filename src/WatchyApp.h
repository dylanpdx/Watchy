#ifndef WATCHY_APP_H
#define WATCHY_APP_H
#include "Watchy.h"

#define BUZZ_APP 1

class WatchyApp {
public:
    WatchyApp(Watchy *watchy) : watchy(watchy) {
        this->watchy = watchy;
    }
    ~WatchyApp() {
        this->destroy();
    }
    virtual void init(){}; // First method to be called when app is selected
    virtual void render(){}; // Update display
    virtual void destroy(){}; // Called when app is exited; Should clean up memory and be able to be called multiple times
    virtual void handleButtonPress(WatchyButton button){}; // Handle button press events
    void exitApp(){watchy->onAppExit();}; // Exits the app and sends an event to the Watchy object
    static WatchyApp *getApp(uint8_t appId, Watchy *watchy); // Returns an instance of the app based on the appId
protected:
    Watchy *watchy;
};

#endif