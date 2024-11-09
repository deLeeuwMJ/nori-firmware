#include "esp32-hal.h"
#include "smd.h"

void SMDPeripheral::setRedValue(int brightness) {
  ledcWrite(RED_LED_CH, brightness);
}

void SMDPeripheral::init() {
  ledcAttachPin(RED_LED_PIN, RED_LED_CH);
  ledcSetup(RED_LED_CH, LED_FREQ, LED_RESOLUTION);
}