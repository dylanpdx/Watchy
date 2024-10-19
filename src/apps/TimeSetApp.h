#ifndef APP_TIMESET_H
#define APP_TIMESET_H
#include "../WatchyApp.h"

class TimeSetApp : public WatchyApp {
public:
    TimeSetApp(Watchy *watchy) : WatchyApp(watchy) {}
    void init() override;
    void destroy() override;
    void handleButtonPress(WatchyButton button) override;
private:
    bool running = true;
    int8_t setIndex = SET_HOUR;
    #ifdef ARDUINO_ESP32S3_DEV
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint8_t year;  
    #else
    int8_t minute;
    int8_t hour;
    int8_t day;
    int8_t month;
    int8_t year;
    #endif
    int8_t blink = 0;
};

#endif