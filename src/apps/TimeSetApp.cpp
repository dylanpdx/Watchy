#include "TimeSetApp.h"


void TimeSetApp::init() {
  watchy->RTC.read(watchy->currentTime);

  #ifdef ARDUINO_ESP32S3_DEV
  minute = watchy->currentTime.Minute;
  hour   = watchy->currentTime.Hour;
  day    = watchy->currentTime.Day;
  month  = watchy->currentTime.Month;
  year   = watchy->currentTime.Year;  
  #else
  minute = watchy->currentTime.Minute;
  hour   = watchy->currentTime.Hour;
  day    = watchy->currentTime.Day;
  month  = watchy->currentTime.Month;
  year   = tmYearToY2k(watchy->currentTime.Year);
  #endif

  pinMode(DOWN_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);

  watchy->display.setFullWindow();

  while (running) {
    watchy->pollButtons();
    blink = 1 - blink;

    watchy->display.fillScreen(GxEPD_BLACK);
    watchy->display.setTextColor(GxEPD_WHITE);
    watchy->display.setFont(&DSEG7_Classic_Bold_53);

    watchy->display.setCursor(5, 80);
    if (setIndex == SET_HOUR) { // blink hour digits
      watchy->display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }
    if (hour < 10) {
      watchy->display.print("0");
    }
    watchy->display.print(hour);

    watchy->display.setTextColor(GxEPD_WHITE);
    watchy->display.print(":");

    watchy->display.setCursor(108, 80);
    if (setIndex == SET_MINUTE) { // blink minute digits
      watchy->display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }
    if (minute < 10) {
      watchy->display.print("0");
    }
    watchy->display.print(minute);

    watchy->display.setTextColor(GxEPD_WHITE);

    watchy->display.setFont(&FreeMonoBold9pt7b);
    watchy->display.setCursor(45, 150);
    if (setIndex == SET_YEAR) { // blink minute digits
      watchy->display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }
    watchy->display.print(2000 + year);

    watchy->display.setTextColor(GxEPD_WHITE);
    watchy->display.print("/");

    if (setIndex == SET_MONTH) { // blink minute digits
      watchy->display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }
    if (month < 10) {
      watchy->display.print("0");
    }
    watchy->display.print(month);

    watchy->display.setTextColor(GxEPD_WHITE);
    watchy->display.print("/");

    if (setIndex == SET_DAY) { // blink minute digits
      watchy->display.setTextColor(blink ? GxEPD_WHITE : GxEPD_BLACK);
    }
    if (day < 10) {
      watchy->display.print("0");
    }
    watchy->display.print(day);
    watchy->display.display(true); // partial refresh
  }

  tmElements_t tm;
  tm.Month  = month;
  tm.Day    = day;
  #ifdef ARDUINO_ESP32S3_DEV
  tm.Year   = year;
  #else
  tm.Year   = y2kYearToTm(year);
  #endif
  tm.Hour   = hour;
  tm.Minute = minute;
  tm.Second = 0;

  watchy->RTC.set(tm);
  exitApp();
}


void TimeSetApp::handleButtonPress(WatchyButton button) {
  switch (button) {
    case WatchyButton::MENU:
      setIndex++;
      if (setIndex > SET_DAY) {
        running = false;
      }
      break;
    case WatchyButton::BACK:
        if (setIndex != SET_HOUR) {
            setIndex--;
        }
        break;
    case WatchyButton::UP:
        blink = 1;
      switch (setIndex) {
      case SET_HOUR:
        hour == 0 ? (hour = 23) : hour--;
        break;
      case SET_MINUTE:
        minute == 0 ? (minute = 59) : minute--;
        break;
      case SET_YEAR:
        year == 0 ? (year = 99) : year--;
        break;
      case SET_MONTH:
        month == 1 ? (month = 12) : month--;
        break;
      case SET_DAY:
        day == 1 ? (day = 31) : day--;
        break;
      default:
        break;
      }
        break;
    case WatchyButton::DOWN:
        blink = 1;
      switch (setIndex) {
        case SET_HOUR:
            hour == 23 ? (hour = 0) : hour++;
            break;
        case SET_MINUTE:
            minute == 59 ? (minute = 0) : minute++;
            break;
        case SET_YEAR:
            year == 99 ? (year = 0) : year++;
            break;
        case SET_MONTH:
            month == 12 ? (month = 1) : month++;
            break;
        case SET_DAY:
            day == 31 ? (day = 1) : day++;
            break;
        default:
            break;
        }
        break;
    }
}

void TimeSetApp::destroy() {
  // n/a
}