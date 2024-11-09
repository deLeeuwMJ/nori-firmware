#include <Arduino.h>
#include <iostream>
#include "esp32-hal.h"

#define LED_FREQ 12000
#define LED_RESOLUTION 8

#define RED_LED_PIN 21
#define GREEN_LED_PIN 22
#define BLUE_LED_PIN 23

uint8_t color = 0;         // a value from 0 to 255 representing the hue
uint32_t R, G, B;          // the Red Green and Blue color components
uint8_t brightness = 255;  // 255 is maximum brightness, but can be changed.  Might need 256 for common anode to fully turn off.

void hueToRGB(uint8_t hue, uint8_t brightness) {
  uint16_t scaledHue = (hue * 6);
  uint8_t segment = scaledHue / 256;                     // segment 0 to 5 around the
                                                         // color wheel
  uint16_t segmentOffset = scaledHue - (segment * 256);  // position within the segment

  uint8_t complement = 0;
  uint16_t prev = (brightness * (255 - segmentOffset)) / 256;
  uint16_t next = (brightness * segmentOffset) / 256;

  brightness = 255 - brightness;
  complement = 255;
  prev = 255 - prev;
  next = 255 - next;

  switch (segment) {
    case 0:  // red
      R = brightness;
      G = next;
      B = complement;
      break;
    case 1:  // yellow
      R = prev;
      G = brightness;
      B = complement;
      break;
    case 2:  // green
      R = complement;
      G = brightness;
      B = next;
      break;
    case 3:  // cyan
      R = complement;
      G = prev;
      B = brightness;
      break;
    case 4:  // blue
      R = next;
      G = complement;
      B = brightness;
      break;
    case 5:  // magenta
    default:
      R = brightness;
      G = complement;
      B = prev;
      break;
  }
}



void setup() {
  std::cout << "Setting up modules..." << std::endl;

  ledcAttachPin(RED_LED_PIN, 0);
  ledcSetup(0, LED_FREQ, LED_RESOLUTION);

  ledcAttachPin(GREEN_LED_PIN, 1);
  ledcSetup(1, LED_FREQ, LED_RESOLUTION);

  ledcAttachPin(BLUE_LED_PIN, 2);
  ledcSetup(2, LED_FREQ, LED_RESOLUTION);
}

void loop() {
  for (color = 0; color < 255; color++) {

    hueToRGB(color, brightness);

    ledcWrite(0, R);
    ledcWrite(1, G);
    ledcWrite(2, B);

    delay(100);
  }

  std::cout << "Next cycle..." << std::endl;
}