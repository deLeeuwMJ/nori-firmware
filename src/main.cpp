#include <Arduino.h>
#include <iostream>
#include "esp32-hal.h"

#define LED_FREQ 12000
#define LED_RESOLUTION 8
#define LED_BRIGHTNESS 200

#define RED_LED_PIN 21
#define RED_LED_CH 0

#define GREEN_LED_PIN 22
#define GREEN_LED_CH 1

#define BLUE_LED_PIN 23
#define BLUE_LED_CH 2

void setRedValue(int brightness) {
  ledcWrite(RED_LED_CH, brightness);
}

void setGreenValue(int brightness) {
  ledcWrite(GREEN_LED_CH, brightness);
}

void setBlueValue(int brightness) {
  ledcWrite(BLUE_LED_CH, brightness);

}

void clearLEDs() {
  setRedValue(0);
  setGreenValue(0);
  setBlueValue(0);
}

void initLEDs() {
  std::cout << "Loaded [initLEDs()]" << std::endl;
  ledcAttachPin(RED_LED_PIN, RED_LED_CH);
  ledcSetup(RED_LED_CH, LED_FREQ, LED_RESOLUTION);

  ledcAttachPin(GREEN_LED_PIN, GREEN_LED_CH);
  ledcSetup(GREEN_LED_CH, LED_FREQ, LED_RESOLUTION);

  ledcAttachPin(BLUE_LED_PIN, BLUE_LED_CH);
  ledcSetup(BLUE_LED_CH, LED_FREQ, LED_RESOLUTION);

  clearLEDs();
}

void initBuzzer() {
  std::cout << "Loaded [initBuzzer()]" << std::endl;
}

void setup() {
  initLEDs();
  initBuzzer();
}

void loop() {
  clearLEDs();
  setRedValue(LED_BRIGHTNESS);
  delay(2500);

  clearLEDs();
  setGreenValue(LED_BRIGHTNESS);
  delay(2500);

  clearLEDs();
  setBlueValue(LED_BRIGHTNESS);
  delay(2500);

  std::cout << "Next cycle..." << std::endl;
}