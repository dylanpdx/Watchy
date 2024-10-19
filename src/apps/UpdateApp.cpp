#include "UpdateApp.h"

void UpdateApp::init() {
  watchy->display.setFullWindow();
  watchy->display.fillScreen(GxEPD_BLACK);
  watchy->display.setFont(&FreeMonoBold9pt7b);
  watchy->display.setTextColor(GxEPD_WHITE);
  watchy->display.setCursor(0, 30);
  watchy->display.println("Please visit");
  watchy->display.println("watchy.sqfmi.com");
  watchy->display.println("with a Bluetooth");
  watchy->display.println("enabled device");
  watchy->display.println(" ");
  watchy->display.println("Press menu button");
  watchy->display.println("again when ready");
  watchy->display.println(" ");
  watchy->display.println("Keep USB powered");
  watchy->display.display(false); // full refresh
}

void UpdateApp::updateFWBegin() {
    updating = true;
  watchy->display.setFullWindow();
  watchy->display.fillScreen(GxEPD_BLACK);
  watchy->display.setFont(&FreeMonoBold9pt7b);
  watchy->display.setTextColor(GxEPD_WHITE);
  watchy->display.setCursor(0, 30);
  watchy->display.println("Bluetooth Started");
  watchy->display.println(" ");
  watchy->display.println("Watchy BLE OTA");
  watchy->display.println(" ");
  watchy->display.println("Waiting for");
  watchy->display.println("connection...");
  watchy->display.display(false); // full refresh

  BLE BT;
  BT.begin("Watchy BLE OTA");
  int prevStatus = -1;
  int currentStatus;

  while (1) {
    currentStatus = BT.updateStatus();
    if (prevStatus != currentStatus || prevStatus == 1) {
      if (currentStatus == 0) {
        watchy->display.setFullWindow();
        watchy->display.fillScreen(GxEPD_BLACK);
        watchy->display.setFont(&FreeMonoBold9pt7b);
        watchy->display.setTextColor(GxEPD_WHITE);
        watchy->display.setCursor(0, 30);
        watchy->display.println("BLE Connected!");
        watchy->display.println(" ");
        watchy->display.println("Waiting for");
        watchy->display.println("upload...");
        watchy->display.display(false); // full refresh
      }
      if (currentStatus == 1) {
        watchy->display.setFullWindow();
        watchy->display.fillScreen(GxEPD_BLACK);
        watchy->display.setFont(&FreeMonoBold9pt7b);
        watchy->display.setTextColor(GxEPD_WHITE);
        watchy->display.setCursor(0, 30);
        watchy->display.println("Downloading");
        watchy->display.println("firmware:");
        watchy->display.println(" ");
        watchy->display.print(BT.howManyBytes());
        watchy->display.println(" bytes");
        watchy->display.display(true); // partial refresh
      }
      if (currentStatus == 2) {
        watchy->display.setFullWindow();
        watchy->display.fillScreen(GxEPD_BLACK);
        watchy->display.setFont(&FreeMonoBold9pt7b);
        watchy->display.setTextColor(GxEPD_WHITE);
        watchy->display.setCursor(0, 30);
        watchy->display.println("Download");
        watchy->display.println("completed!");
        watchy->display.println(" ");
        watchy->display.println("Rebooting...");
        watchy->display.display(false); // full refresh

        delay(2000);
        esp_restart();
      }
      if (currentStatus == 4) {
        watchy->display.setFullWindow();
        watchy->display.fillScreen(GxEPD_BLACK);
        watchy->display.setFont(&FreeMonoBold9pt7b);
        watchy->display.setTextColor(GxEPD_WHITE);
        watchy->display.setCursor(0, 30);
        watchy->display.println("BLE Disconnected!");
        watchy->display.println(" ");
        watchy->display.println("exiting...");
        watchy->display.display(false); // full refresh
        delay(1000);
        break;
      }
      prevStatus = currentStatus;
    }
    delay(100);
  }

  // turn off radios
  WiFi.mode(WIFI_OFF);
  btStop();
    exitApp();
}

void UpdateApp::destroy() {
    // n/a
}

void UpdateApp::handleButtonPress(WatchyButton button) {
    if (updating) {
        return;
    }
    if (button == WatchyButton::MENU) {
        updateFWBegin();
    }else if (button == WatchyButton::BACK) {
        exitApp();
    }
}