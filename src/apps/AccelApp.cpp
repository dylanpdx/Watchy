#include "AccelApp.h"

void AccelApp::init() {
    watchy->display.setFullWindow();
    watchy->display.fillScreen(GxEPD_BLACK);
    watchy->display.setFont(&FreeMonoBold9pt7b);
    watchy->display.setTextColor(GxEPD_WHITE);

    Accel acc;

    long previousMillis = 0;
    long interval       = 200;

    guiState = APP_STATE;

    while (running) {
        watchy->pollButtons();
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis > interval) {
            previousMillis = currentMillis;
            // Get acceleration data
            bool res          = sensor.getAccel(acc);
            uint8_t direction = sensor.getDirection();
            watchy->display.fillScreen(GxEPD_BLACK);
            watchy->display.setCursor(0, 30);
            if (res == false) {
            watchy->display.println("getAccel FAIL");
            } else {
            watchy->display.print("  X:");
            watchy->display.println(acc.x);
            watchy->display.print("  Y:");
            watchy->display.println(acc.y);
            watchy->display.print("  Z:");
            watchy->display.println(acc.z);

            watchy->display.setCursor(30, 130);
            switch (direction) {
            case DIRECTION_DISP_DOWN:
                watchy->display.println("FACE DOWN");
                break;
            case DIRECTION_DISP_UP:
                watchy->display.println("FACE UP");
                break;
            case DIRECTION_BOTTOM_EDGE:
                watchy->display.println("BOTTOM EDGE");
                break;
            case DIRECTION_TOP_EDGE:
                watchy->display.println("TOP EDGE");
                break;
            case DIRECTION_RIGHT_EDGE:
                watchy->display.println("RIGHT EDGE");
                break;
            case DIRECTION_LEFT_EDGE:
                watchy->display.println("LEFT EDGE");
                break;
            default:
                watchy->display.println("ERROR!!!");
                break;
            }
            }
            watchy->display.display(true); // full refresh
        }
    }
    exitApp();
}


void AccelApp::destroy() {
    // n/a
}

void AccelApp::handleButtonPress(WatchyButton button) {
    if (button == WatchyButton::BACK) {
        running = false;
    }
}