#include "WifiSetupApp.h"

void WifiSetupApp::_configModeCallback(WiFiManager *myWiFiManager) {
  Watchy::display.setFullWindow();
  Watchy::display.fillScreen(GxEPD_BLACK);
  Watchy::display.setFont(&FreeMonoBold9pt7b);
  Watchy::display.setTextColor(GxEPD_WHITE);
  Watchy::display.setCursor(0, 30);
  Watchy::display.println("Connect to");
  Watchy::display.print("SSID: ");
  Watchy::display.println(WIFI_AP_SSID);
  Watchy::display.print("IP: ");
  Watchy::display.println(WiFi.softAPIP());
	Watchy::display.println("MAC address:");
	Watchy::display.println(WiFi.softAPmacAddress().c_str());
  Watchy::display.display(false); // full refresh
}

void WifiSetupApp::init() {
  watchy->display.epd2.setBusyCallback(0); // temporarily disable lightsleep on busy
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.setTimeout(WIFI_AP_TIMEOUT);
  wifiManager.setAPCallback(_configModeCallback);
  watchy->display.setFullWindow();
  watchy->display.fillScreen(GxEPD_BLACK);
  watchy->display.setFont(&FreeMonoBold9pt7b);
  watchy->display.setTextColor(GxEPD_WHITE);
  if (!wifiManager.autoConnect(WIFI_AP_SSID)) { // WiFi setup failed
    watchy->display.println("Setup failed &");
    watchy->display.println("timed out!");
  } else {
    watchy->display.println("Connected to:");
    watchy->display.println(WiFi.SSID());
		watchy->display.println("Local IP:");
		watchy->display.println(WiFi.localIP());
    watchy->UpdateWifi();
  }
  watchy->display.display(false); // full refresh
  // turn off radios
  WiFi.mode(WIFI_OFF);
  btStop();
  // enable lightsleep on busy
  watchy->display.epd2.setBusyCallback(WatchyDisplay::busyCallback);
  exitApp();
}

void WifiSetupApp::destroy() {
    // n/a
}

void WifiSetupApp::handleButtonPress(WatchyButton button) {
    // n/a
}