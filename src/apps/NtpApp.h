#ifndef APP_NTP_H
#define APP_NTP_H
#include "../WatchyApp.h"

class NtpApp : public WatchyApp {
public:
    NtpApp(Watchy *watchy) : WatchyApp(watchy) {}
    void init() override;
};

#endif