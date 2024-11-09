#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "smd.h"
#include "buzzer.h"

#define SCAN_TIME 10
#define OBD2_ADDRESS "75:0E:4A:ED:E9:60"

auto smd = SMDPeripheral();
auto buzzer = BuzzerPeripheral();

int generateSpeedValue() {
  return rand() % 105;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
};

BLEScan *pBLEScan;

void setup() {
  Serial.begin(115200);
  smd.init();

  Serial.println("Scanning...");

  BLEDevice::init("Nori091124");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  // auto current_speed = generateSpeedValue();
  // Serial.printf("Current Speed: %s \n", current_speed);

  // if (current_speed > 100) {
  //   smd.setRedValue(LED_BRIGHTNESS);
  //   delay(100);
  //   buzzer.playTooHighSpeed();
  // } else {
  //   smd.setRedValue(0);
  // }
  auto foundDevices = pBLEScan->start(SCAN_TIME, false);
  Serial.println("Scan done!");

  delay(1000);

  for (int i = 0; i < foundDevices.getCount() - 1; i++) {
    auto device = foundDevices.getDevice(i);

    if (device.getName() == "DEV-BLE-PRP"){
      Serial.print("Device found!");
    }
  }

  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory


  delay(5000);
}