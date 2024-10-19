#include "NtpApp.h"

void NtpApp::init() {
  watchy->display.setFullWindow();
  watchy->display.fillScreen(GxEPD_BLACK);
  watchy->display.setFont(&FreeMonoBold9pt7b);
  watchy->display.setTextColor(GxEPD_WHITE);
  watchy->display.setCursor(0, 30);
  watchy->display.println("Syncing NTP... ");
  watchy->display.print("GMT offset: ");
  watchy->display.println(watchy->getLocalTimeOffset());
  watchy->display.display(false); // full refresh
  if (watchy->connectWiFi()) {
    if (watchy->syncNTP()) {
      watchy->display.println("NTP Sync Success\n");
      watchy->display.println("Current Time Is:");

      watchy->RTC.read(watchy->currentTime);

      watchy->display.print(tmYearToCalendar(watchy->currentTime.Year));
      watchy->display.print("/");
      watchy->display.print(watchy->currentTime.Month);
      watchy->display.print("/");
      watchy->display.print(watchy->currentTime.Day);
      watchy->display.print(" - ");

      if (watchy->currentTime.Hour < 10) {
        watchy->display.print("0");
      }
      watchy->display.print(watchy->currentTime.Hour);
      watchy->display.print(":");
      if (watchy->currentTime.Minute < 10) {
        watchy->display.print("0");
      }
      watchy->display.println(watchy->currentTime.Minute);
    } else {
      watchy->display.println("NTP Sync Failed");
    }
  } else {
    watchy->display.println("WiFi Not Configured");
  }
  watchy->display.display(true); // full refresh
  delay(3000);
    exitApp();
}