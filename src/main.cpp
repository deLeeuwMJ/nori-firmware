#include <Arduino.h>
#include <iostream>

#include "smd.h"
#include "buzzer.h"

auto smd = SMDPeripheral();
auto buzzer = BuzzerPeripheral();

int generateSpeedValue() {
  return rand() % 105;
}

void setup() {
  smd.init();
}

void loop() {
  auto current_speed = generateSpeedValue();
  std::cout << "Current Speed: " << current_speed << std::endl;

  if (current_speed > 100) {
    smd.setRedValue(LED_BRIGHTNESS);
    delay(100);
    buzzer.playTooHighSpeed();
  } else {
    smd.setRedValue(0);
  }

  delay(2500);
}