#include "AboutApp.h"

void AboutApp::init() {
    render();
}

void AboutApp::render() {
    watchy->display.setFullWindow();
    watchy->display.fillScreen(GxEPD_BLACK);
    watchy->display.setFont(&FreeMonoBold9pt7b);
    watchy->display.setTextColor(GxEPD_WHITE);
    watchy->display.setCursor(0, 20);

    watchy->display.print("LibVer: ");
    watchy->display.println(WATCHY_LIB_VER);

    watchy->display.print("Rev: v");
    watchy->display.println(watchy->getBoardRevision());

    watchy->display.print("Batt: ");
    float voltage = watchy->getBatteryVoltage();
    watchy->display.print(voltage);
    watchy->display.println("V");

    #ifndef ARDUINO_ESP32S3_DEV
    watchy->display.print("Uptime: ");
    RTC.read(currentTime);
    time_t b = makeTime(bootTime);
    time_t c = makeTime(currentTime);
    int totalSeconds = c-b;
    //int seconds = (totalSeconds % 60);
    int minutes = (totalSeconds % 3600) / 60;
    int hours = (totalSeconds % 86400) / 3600;
    int days = (totalSeconds % (86400 * 30)) / 86400; 
    watchy->display.print(days);
    watchy->display.print("d");
    watchy->display.print(hours);
    watchy->display.print("h");
    watchy->display.print(minutes);
    watchy->display.println("m");  
    #endif

    /*if(WIFI_CONFIGURED){
    watchy->display.print("SSID: ");
    watchy->display.println(lastSSID);
    watchy->display.print("IP: ");
    watchy->display.println(IPAddress(lastIPAddress).toString());
    }else{
    watchy->display.println("WiFi Not Connected");
    }*/
   
    watchy->display.display(false); // full refresh
}

void AboutApp::destroy() {
    // n/a
}

void AboutApp::handleButtonPress(WatchyButton button) {
    if (button == WatchyButton::BACK) {
        exitApp();
    }
}