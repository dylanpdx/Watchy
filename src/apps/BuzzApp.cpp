#include "BuzzApp.h"

void BuzzApp::init() {
    render();
    watchy->vibMotor();
    exitApp();
}

void BuzzApp::render() {
    watchy->display.setFullWindow();
    watchy->display.fillScreen(GxEPD_BLACK);
    watchy->display.setFont(&FreeMonoBold9pt7b);
    watchy->display.setTextColor(GxEPD_WHITE);
    watchy->display.setCursor(70, 80);
    watchy->display.println("Buzz!");
    watchy->display.display(false); // full refresh
}

void BuzzApp::destroy() {
    // n/a
}

void BuzzApp::handleButtonPress(WatchyButton button) {
    // n/a
}