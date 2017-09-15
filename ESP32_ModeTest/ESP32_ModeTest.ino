// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Sketch shows how to switch between WiFi and BlueTooth or use both
// Button is attached between GPIO 0 and GND and modes are switched with each press

// Adapted by Andreas Spiess 2017

#include "WiFi.h"
#define STA_SSID "xxx"
#define STA_PASS "xxx"
#define AP_SSID  "esp32"

#define SWITCHPIN 4

enum { STEP_STA,STEP_BTON, STEP_AP, STEP_AP_STA, STEP_OFF, STEP_BT_STA, STEP_END, STEP_SLEEP };

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  15        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0;

void onButton() {
  static uint32_t step = STEP_BTON;
  switch (step) {
    case STEP_BTON://BT Only
      Serial.println("** Starting BT");
      btStart();
      break;
    case STEP_STA://STA Only
      Serial.println("** Stopping BT");
      btStop();
      Serial.println("\n** Starting STA");
      WiFi.begin(STA_SSID, STA_PASS);
      break;
    case STEP_AP://AP Only
      Serial.println("** Stopping STA");
      WiFi.mode(WIFI_AP);
      Serial.println("\n** Starting AP");
      WiFi.softAP(AP_SSID);
      break;
    case STEP_AP_STA://AP+STA
      Serial.println("\n** Starting AP + STA");
      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(AP_SSID);
      delay(500);
      WiFi.begin(STA_SSID, STA_PASS);
      break;
    case STEP_OFF://All Off
      Serial.println("** Stopping WiFi");
      WiFi.mode(WIFI_OFF);
      break;
    case STEP_BT_STA://BT+STA
      Serial.println("\n** Starting STA+BT");
      WiFi.begin(STA_SSID, STA_PASS);
      delay(500);
      btStart();
      break;
    case STEP_END://All Off
      Serial.println("** Stopping WiFi+BT");
      WiFi.mode(WIFI_OFF);
      btStop();
      break;
    case STEP_SLEEP:
      Serial.println("\n** Going to sleep now");
      esp_deep_sleep_start();
      break;
    default:
      break;
  }
  if (step == STEP_SLEEP) {
    step = STEP_BTON;
  } else {
    step++;
  }
  //little debounce
  delay(100);
}

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_AP_START:
      Serial.println("AP Started");
      WiFi.softAPsetHostname(AP_SSID);
      break;
    case SYSTEM_EVENT_AP_STOP:
      Serial.println("AP Stopped");
      break;
    case SYSTEM_EVENT_STA_START:
      Serial.println("STA Started");
      WiFi.setHostname(AP_SSID);
      break;
    case SYSTEM_EVENT_STA_CONNECTED:
      Serial.println("STA Connected");
      WiFi.enableIpV6();
      break;
    case SYSTEM_EVENT_AP_STA_GOT_IP6:
      Serial.print("STA IPv6: ");
      Serial.println(WiFi.localIPv6());
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.print("STA IPv4: ");
      Serial.println(WiFi.localIP());
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("STA Disconnected");
      break;
    case SYSTEM_EVENT_STA_STOP:
      Serial.println("STA Stopped");
      break;
    default:
      break;
  }
}

void print_wakeup_reason() {
  esp_deep_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_deep_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  esp_deep_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");

  pinMode(SWITCHPIN, INPUT_PULLUP);
  WiFi.onEvent(WiFiEvent);
  Serial.print("ESP32 SDK: ");
  Serial.println(ESP.getSdkVersion());
  Serial.println("Press the button to select the next mode");
}

void loop() {
  static uint8_t lastPinState = 1;
  uint8_t pinState = digitalRead(SWITCHPIN);
  if (!pinState && lastPinState) {
    onButton();
  }
  lastPinState = pinState;
  delay(100);
}
