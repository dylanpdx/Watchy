#include "WatchyApp.h"
#include "apps/AboutApp.h"
#include "apps/BuzzApp.h"
#include "apps/AccelApp.h"
#include "apps/TimeSetApp.h"
#include "apps/WifiSetupApp.h"
#include "apps/UpdateApp.h"
#include "apps/NtpApp.h"

WatchyApp *WatchyApp::getApp(uint8_t appId, Watchy *watchy) {
    switch (appId) {
        case ABOUT_APP:
            return new AboutApp(watchy);
        case BUZZ_APP:
            return new BuzzApp(watchy);
        case ACCEL_APP:
            return new AccelApp(watchy);
        case TIME_SET_APP:
            return new TimeSetApp(watchy);
        case WIFI_SETUP_APP:
            return new WifiSetupApp(watchy);
        case UPDATE_APP:
            return new UpdateApp(watchy);
        case NTP_APP:
            return new NtpApp(watchy);
        default:
            return nullptr;
    }
}