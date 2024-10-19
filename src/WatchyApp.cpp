#include "WatchyApp.h"
#include "apps/BuzzApp.h"

WatchyApp *WatchyApp::getApp(uint8_t appId, Watchy *watchy) {
    switch (appId) {
        case BUZZ_APP:
            return new BuzzApp(watchy);
        default:
            return nullptr;
    }
}