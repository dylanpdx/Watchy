#include "Watchy.h"
#include "Menu.h"
#include "WatchyApp.h"

#ifdef ARDUINO_ESP32S3_DEV
  Watchy32KRTC Watchy::RTC;
  #define ACTIVE_LOW 0
#else
  WatchyRTC Watchy::RTC;
  #define ACTIVE_LOW 1
#endif
GxEPD2_BW<WatchyDisplay, WatchyDisplay::HEIGHT> Watchy::display(
    WatchyDisplay{});

RTC_DATA_ATTR int guiState;
RTC_DATA_ATTR BMA423 sensor;
RTC_DATA_ATTR bool WIFI_CONFIGURED;
RTC_DATA_ATTR bool BLE_CONFIGURED;
RTC_DATA_ATTR weatherData currentWeather;
RTC_DATA_ATTR int weatherIntervalCounter = -1;
RTC_DATA_ATTR long gmtOffset = 0;
RTC_DATA_ATTR bool alreadyInMenu         = true;
RTC_DATA_ATTR bool USB_PLUGGED_IN = false;
RTC_DATA_ATTR tmElements_t bootTime;
RTC_DATA_ATTR uint32_t lastIPAddress;
RTC_DATA_ATTR char lastSSID[30];
Menu *watchyMenu=NULL;
WatchyApp *currentApp = NULL;

void Watchy::init(String datetime) {
  if (watchyMenu == NULL) {
    watchyMenu = new Menu(this);
  }
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause(); // get wake up reason
  #ifdef ARDUINO_ESP32S3_DEV
    Wire.begin(WATCHY_V3_SDA, WATCHY_V3_SCL);     // init i2c
  #else
    Wire.begin(SDA, SCL);                         // init i2c
  #endif
  RTC.init();
  // Init the display since is almost sure we will use it
  display.epd2.initWatchy();

  switch (wakeup_reason) {
  #ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_TIMER: // RTC Alarm
  #else
  case ESP_SLEEP_WAKEUP_EXT0: // RTC Alarm
  #endif
    RTC.read(currentTime);
    switch (guiState) {
    case WATCHFACE_STATE:
      showWatchFace(true); // partial updates on tick
      if (settings.vibrateOClock) {
        if (currentTime.Minute == 0) {
          // The RTC wakes us up once per minute
          vibMotor(75, 4);
        }
      }
      break;
    case MAIN_MENU_STATE:
      // Return to watchface if in menu for more than one tick
      if (alreadyInMenu) {
        guiState = WATCHFACE_STATE;
        showWatchFace(false);
      } else {
        alreadyInMenu = true;
      }
      break;
    }
    break;
  case ESP_SLEEP_WAKEUP_EXT1: // button Press
    Watchy::handleButtonWakeup();
    break;
  #ifdef ARDUINO_ESP32S3_DEV
  case ESP_SLEEP_WAKEUP_EXT0: // USB plug in
    pinMode(USB_DET_PIN, INPUT);
    USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
    if(guiState == WATCHFACE_STATE){
      RTC.read(currentTime);
      showWatchFace(true);
    }
    break;
  #endif
  default: // reset
    RTC.config(datetime);
    _bmaConfig();
    #ifdef ARDUINO_ESP32S3_DEV
    pinMode(USB_DET_PIN, INPUT);
    USB_PLUGGED_IN = (digitalRead(USB_DET_PIN) == 1);
    #endif    
    gmtOffset = settings.gmtOffset;
    RTC.read(currentTime);
    RTC.read(bootTime);
    showWatchFace(false); // full update on reset
    vibMotor(75, 4);
    // For some reason, seems to be enabled on first boot
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    break;
  }
  deepSleep();
}
void Watchy::deepSleep() {
  display.hibernate();
  RTC.clearAlarm();        // resets the alarm flag in the RTC
  #ifdef ARDUINO_ESP32S3_DEV
  esp_sleep_enable_ext0_wakeup((gpio_num_t)USB_DET_PIN, USB_PLUGGED_IN ? LOW : HIGH); //// enable deep sleep wake on USB plug in/out
  rtc_gpio_set_direction((gpio_num_t)USB_DET_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)USB_DET_PIN);

  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_LOW); // enable deep sleep wake on button press
  rtc_gpio_set_direction((gpio_num_t)UP_BTN_PIN, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)UP_BTN_PIN);

  rtc_clk_32k_enable(true);
  //rtc_clk_slow_freq_set(RTC_SLOW_FREQ_32K_XTAL);
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  int secToNextMin = 60 - timeinfo.tm_sec;
  esp_sleep_enable_timer_wakeup(secToNextMin * uS_TO_S_FACTOR);
  #else
  // Set GPIOs 0-39 to input to avoid power leaking out
  const uint64_t ignore = 0b11110001000000110000100111000010; // Ignore some GPIOs due to resets
  for (int i = 0; i < GPIO_NUM_MAX; i++) {
    if ((ignore >> i) & 0b1)
      continue;
    pinMode(i, INPUT);
  }
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RTC_INT_PIN,
                               0); // enable deep sleep wake on RTC interrupt
  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_HIGH); // enable deep sleep wake on button press
  #endif
  esp_deep_sleep_start();
}

void Watchy::handleButtonWakeup() {
  WatchyButton button = WatchyButton::NONE;
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
  if (wakeupBit & MENU_BTN_MASK) {
    button = WatchyButton::MENU;
  } else if (wakeupBit & BACK_BTN_MASK) {
    button = WatchyButton::BACK;
  } else if (wakeupBit & UP_BTN_MASK) {
    button = WatchyButton::UP;
  } else if (wakeupBit & DOWN_BTN_MASK) {
    button = WatchyButton::DOWN;
  }
  handleButtonPress(button);
  // we're awake now, stay awake for a bit
  bool timeout     = false;
  long lastTimeout = millis();
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(DOWN_BTN_PIN, INPUT);
  while (!timeout){
    if (millis() - lastTimeout > 5000) {
      timeout = true;
    } else {
      if (pollButtons()) {
        lastTimeout = millis();
      }
    }
  }
}

bool Watchy::pollButtons(){
    pinMode(MENU_BTN_PIN, INPUT);
    pinMode(BACK_BTN_PIN, INPUT);
    pinMode(UP_BTN_PIN, INPUT);
    pinMode(DOWN_BTN_PIN, INPUT);

    if (digitalRead(MENU_BTN_PIN) == ACTIVE_LOW) {
      handleButtonPress(WatchyButton::MENU);
      return true;
    } else if (digitalRead(BACK_BTN_PIN) == ACTIVE_LOW) {
      handleButtonPress(WatchyButton::BACK);
      return true;
    } else if (digitalRead(UP_BTN_PIN) == ACTIVE_LOW) {
      handleButtonPress(WatchyButton::UP);
      return true;
    } else if (digitalRead(DOWN_BTN_PIN) == ACTIVE_LOW) {
      handleButtonPress(WatchyButton::DOWN);
      return true;
    }
    return false;
}

void Watchy::handleButtonPress(WatchyButton buttonPressed) {

  // if in app, handle button press in app
  if (guiState == APP_STATE && currentApp != NULL) {
    currentApp->handleButtonPress(buttonPressed);
    return;
  }

  // Menu Button
  if (buttonPressed == WatchyButton::MENU) {
    if (guiState == WATCHFACE_STATE) { // enter menu state if coming from watch face
      showMenu(false);
    } else if (guiState == MAIN_MENU_STATE) { // if already in menu, then select menu item
      watchyMenu->getSelectedItem()->executeHandler(*this);
    }
  }
  // Back Button
  else if (buttonPressed == WatchyButton::BACK) {
    if (guiState == MAIN_MENU_STATE) { // exit to watch face if already in menu
      RTC.read(currentTime);
      showWatchFace(false);
    } else if (guiState == WATCHFACE_STATE) {
      return;
    }
  }
  // Up Button
  else if (buttonPressed == WatchyButton::UP) {
    if (guiState == MAIN_MENU_STATE) { // increment menu index
      watchyMenu->moveCursor(MENU_DIR_UP);
      showMenu(true);
    } else if (guiState == WATCHFACE_STATE) {
      return;
    }
  }
  // Down Button
  else if (buttonPressed == WatchyButton::DOWN) {
    if (guiState == MAIN_MENU_STATE) { // decrement menu index
      watchyMenu->moveCursor(MENU_DIR_DOWN);
      showMenu(true);
    } else if (guiState == WATCHFACE_STATE) {
      return;
    }
  }
}

void Watchy::showMenu(bool partialRefresh) {
  watchyMenu->renderMenu(partialRefresh);

  guiState = MAIN_MENU_STATE;
  alreadyInMenu = false;
}

void Watchy::showFastMenu() {
  watchyMenu->renderMenu(true);

  guiState = MAIN_MENU_STATE;
}

void Watchy::showAbout() {

}

void Watchy::vibMotor(uint8_t intervalMs, uint8_t length) {
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  bool motorOn = false;
  for (int i = 0; i < length; i++) {
    motorOn = !motorOn;
    digitalWrite(VIB_MOTOR_PIN, motorOn);
    delay(intervalMs);
  }
}

void Watchy::showWatchFace(bool partialRefresh) {
  display.setFullWindow();
  // At this point it is sure we are going to update
  display.epd2.asyncPowerOn();
  drawWatchFace();
  display.display(partialRefresh); // partial refresh
  guiState = WATCHFACE_STATE;
}

void Watchy::drawWatchFace() {
  display.setFont(&DSEG7_Classic_Bold_53);
  display.setCursor(5, 53 + 60);
  if (currentTime.Hour < 10) {
    display.print("0");
  }
  display.print(currentTime.Hour);
  display.print(":");
  if (currentTime.Minute < 10) {
    display.print("0");
  }
  display.println(currentTime.Minute);
}

weatherData Watchy::getWeatherData() {
  return _getWeatherData(settings.cityID, settings.lat, settings.lon,
    settings.weatherUnit, settings.weatherLang, settings.weatherURL,
    settings.weatherAPIKey, settings.weatherUpdateInterval);
}

weatherData Watchy::_getWeatherData(String cityID, String lat, String lon, String units, String lang,
                                   String url, String apiKey,
                                   uint8_t updateInterval) {
  currentWeather.isMetric = units == String("metric");
  if (weatherIntervalCounter < 0) { //-1 on first run, set to updateInterval
    weatherIntervalCounter = updateInterval;
  }
  if (weatherIntervalCounter >=
      updateInterval) { // only update if WEATHER_UPDATE_INTERVAL has elapsed
                        // i.e. 30 minutes
    if (connectWiFi()) {
      HTTPClient http; // Use Weather API for live data if WiFi is connected
      http.setConnectTimeout(3000); // 3 second max timeout
      String weatherQueryURL = url;
      if(cityID != ""){
        weatherQueryURL.replace("{cityID}", cityID);
      }else{
        weatherQueryURL.replace("{lat}", lat);
        weatherQueryURL.replace("{lon}", lon);
      }
      weatherQueryURL.replace("{units}", units);
      weatherQueryURL.replace("{lang}", lang);
      weatherQueryURL.replace("{apiKey}", apiKey);
      http.begin(weatherQueryURL.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode == 200) {
        String payload             = http.getString();
        JSONVar responseObject     = JSON.parse(payload);
        currentWeather.temperature = int(responseObject["main"]["temp"]);
        currentWeather.weatherConditionCode =
            int(responseObject["weather"][0]["id"]);
        currentWeather.weatherDescription =
		        JSONVar::stringify(responseObject["weather"][0]["main"]);
	      currentWeather.external = true;
		        breakTime((time_t)(int)responseObject["sys"]["sunrise"], currentWeather.sunrise);
		        breakTime((time_t)(int)responseObject["sys"]["sunset"], currentWeather.sunset);
        // sync NTP during weather API call and use timezone of lat & lon
        gmtOffset = int(responseObject["timezone"]);
        syncNTP(gmtOffset);
      } else {
        // http error
      }
      http.end();
      // turn off radios
      WiFi.mode(WIFI_OFF);
      btStop();
    } else { // No WiFi, use internal temperature sensor
      uint8_t temperature = sensor.readTemperature(); // celsius
      if (!currentWeather.isMetric) {
        temperature = temperature * 9. / 5. + 32.; // fahrenheit
      }
      currentWeather.temperature          = temperature;
      currentWeather.weatherConditionCode = 800;
      currentWeather.external             = false;
    }
    weatherIntervalCounter = 0;
  } else {
    weatherIntervalCounter++;
  }
  return currentWeather;
}

float Watchy::getBatteryVoltage() {
  #ifdef ARDUINO_ESP32S3_DEV
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * ADC_VOLTAGE_DIVIDER;
  #else
  if (RTC.rtcType == DS3231) {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f *
           2.0f; // Battery voltage goes through a 1/2 divider.
  } else {
    return analogReadMilliVolts(BATT_ADC_PIN) / 1000.0f * 2.0f;
  }
  #endif
}

uint8_t Watchy::getBoardRevision() {
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  if(chip_info.model == CHIP_ESP32){ //Revision 1.0 - 2.0
    Wire.beginTransmission(0x68); //v1.0 has DS3231
    if (Wire.endTransmission() == 0){
      return 10;
    }
    delay(1);
    Wire.beginTransmission(0x51); //v1.5 and v2.0 have PCF8563
    if (Wire.endTransmission() == 0){
        pinMode(35, INPUT);
        if(digitalRead(35) == 0){
          return 20; //in rev 2.0, pin 35 is BTN 3 and has a pulldown
        }else{
          return 15; //in rev 1.5, pin 35 is the battery ADC
        }
    }
  }
  if(chip_info.model == CHIP_ESP32S3){ //Revision 3.0
    return 30;
  }
  return -1;
}

uint16_t Watchy::_readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                               uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom((uint8_t)address, (uint8_t)len);
  uint8_t i = 0;
  while (Wire.available()) {
    data[i++] = Wire.read();
  }
  return 0;
}

uint16_t Watchy::_writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                                uint16_t len) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data, len);
  return (0 != Wire.endTransmission());
}

void Watchy::_bmaConfig() {

  if (sensor.begin(_readRegister, _writeRegister, delay) == false) {
    // fail to init BMA
    return;
  }

  // Accel parameter structure
  Acfg cfg;
  /*!
      Output data rate in Hz, Optional parameters:
          - BMA4_OUTPUT_DATA_RATE_0_78HZ
          - BMA4_OUTPUT_DATA_RATE_1_56HZ
          - BMA4_OUTPUT_DATA_RATE_3_12HZ
          - BMA4_OUTPUT_DATA_RATE_6_25HZ
          - BMA4_OUTPUT_DATA_RATE_12_5HZ
          - BMA4_OUTPUT_DATA_RATE_25HZ
          - BMA4_OUTPUT_DATA_RATE_50HZ
          - BMA4_OUTPUT_DATA_RATE_100HZ
          - BMA4_OUTPUT_DATA_RATE_200HZ
          - BMA4_OUTPUT_DATA_RATE_400HZ
          - BMA4_OUTPUT_DATA_RATE_800HZ
          - BMA4_OUTPUT_DATA_RATE_1600HZ
  */
  cfg.odr = BMA4_OUTPUT_DATA_RATE_100HZ;
  /*!
      G-range, Optional parameters:
          - BMA4_ACCEL_RANGE_2G
          - BMA4_ACCEL_RANGE_4G
          - BMA4_ACCEL_RANGE_8G
          - BMA4_ACCEL_RANGE_16G
  */
  cfg.range = BMA4_ACCEL_RANGE_2G;
  /*!
      Bandwidth parameter, determines filter configuration, Optional parameters:
          - BMA4_ACCEL_OSR4_AVG1
          - BMA4_ACCEL_OSR2_AVG2
          - BMA4_ACCEL_NORMAL_AVG4
          - BMA4_ACCEL_CIC_AVG8
          - BMA4_ACCEL_RES_AVG16
          - BMA4_ACCEL_RES_AVG32
          - BMA4_ACCEL_RES_AVG64
          - BMA4_ACCEL_RES_AVG128
  */
  cfg.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

  /*! Filter performance mode , Optional parameters:
      - BMA4_CIC_AVG_MODE
      - BMA4_CONTINUOUS_MODE
  */
  cfg.perf_mode = BMA4_CONTINUOUS_MODE;

  // Configure the BMA423 accelerometer
  sensor.setAccelConfig(cfg);

  // Enable BMA423 accelerometer
  // Warning : Need to use feature, you must first enable the accelerometer
  // Warning : Need to use feature, you must first enable the accelerometer
  sensor.enableAccel();

  struct bma4_int_pin_config config;
  config.edge_ctrl = BMA4_LEVEL_TRIGGER;
  config.lvl       = BMA4_ACTIVE_HIGH;
  config.od        = BMA4_PUSH_PULL;
  config.output_en = BMA4_OUTPUT_ENABLE;
  config.input_en  = BMA4_INPUT_DISABLE;
  // The correct trigger interrupt needs to be configured as needed
  sensor.setINTPinConfig(config, BMA4_INTR1_MAP);

  struct bma423_axes_remap remap_data;
  remap_data.x_axis      = 1;
  remap_data.x_axis_sign = 0xFF;
  remap_data.y_axis      = 0;
  remap_data.y_axis_sign = 0xFF;
  remap_data.z_axis      = 2;
  remap_data.z_axis_sign = 0xFF;
  // Need to raise the wrist function, need to set the correct axis
  sensor.setRemapAxes(&remap_data);

  // Enable BMA423 isStepCounter feature
  sensor.enableFeature(BMA423_STEP_CNTR, true);
  // Enable BMA423 isTilt feature
  sensor.enableFeature(BMA423_TILT, true);
  // Enable BMA423 isDoubleClick feature
  sensor.enableFeature(BMA423_WAKEUP, true);

  // Reset steps
  sensor.resetStepCounter();

  // Turn on feature interrupt
  sensor.enableStepCountInterrupt();
  sensor.enableTiltInterrupt();
  // It corresponds to isDoubleClick interrupt
  sensor.enableWakeupInterrupt();
}

bool Watchy::connectWiFi() {
  if (WL_CONNECT_FAILED ==
      WiFi.begin()) { // WiFi not setup, you can also use hard coded credentials
                      // with WiFi.begin(SSID,PASS);
    WIFI_CONFIGURED = false;
  } else {
    if (WL_CONNECTED ==
        WiFi.waitForConnectResult()) { // attempt to connect for 10s
      lastIPAddress = WiFi.localIP();
      WiFi.SSID().toCharArray(lastSSID, 30);
      WIFI_CONFIGURED = true;
    } else { // connection failed, time out
      WIFI_CONFIGURED = false;
      // turn off radios
      WiFi.mode(WIFI_OFF);
      btStop();
    }
  }
  return WIFI_CONFIGURED;
}

void Watchy::UpdateWifi(){
  weatherIntervalCounter = -1; // Reset to force weather to be read again
  lastIPAddress = WiFi.localIP();
  WiFi.SSID().toCharArray(lastSSID, 30);
}

RTC_DATA_ATTR long Watchy::getLocalTimeOffset() {
  return gmtOffset;
}

bool Watchy::syncNTP() { // NTP sync - call after connecting to WiFi and
                         // remember to turn it back off
  return syncNTP(gmtOffset,
                 settings.ntpServer.c_str());
}

bool Watchy::syncNTP(long gmt) {
  return syncNTP(gmt, settings.ntpServer.c_str());
}

bool Watchy::syncNTP(long gmt, String ntpServer) {
  // NTP sync - call after connecting to
  // WiFi and remember to turn it back off
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP, ntpServer.c_str(), gmt);
  timeClient.begin();
  if (!timeClient.forceUpdate()) {
    return false; // NTP sync failed
  }
  tmElements_t tm;
  breakTime((time_t)timeClient.getEpochTime(), tm);
  RTC.set(tm);
  return true;
}

void Watchy::closeCurrentApp(){
  if (currentApp != NULL){
    currentApp->destroy();
    // free memory
    delete currentApp;
    currentApp = NULL;
  }
}

void Watchy::onAppExit(){
  closeCurrentApp();
  showMenu(false);
  guiState = MAIN_MENU_STATE;
}

void Watchy::launchApp(uint8_t appId){
  WatchyApp *app = WatchyApp::getApp(appId, this);
  if (app != nullptr && currentApp != app){
    if (currentApp != NULL){
      closeCurrentApp();
    }
    currentApp = app;
    guiState = APP_STATE;
    app->init();
  }
}