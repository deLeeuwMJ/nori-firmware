#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "smd.h"
#include "buzzer.h"

#define SCAN_TIME 10

static BLEUUID serviceUUID("180D");
static BLEUUID charUUID("2A38");

auto smd = SMDPeripheral();
auto buzzer = BuzzerPeripheral();

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.printf("Advertised Device: %s \n", advertisedDevice.getAddress().toString().c_str());
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
  auto foundDevices = pBLEScan->start(SCAN_TIME, false);
  Serial.println("Scan done!");

  delay(1000);

  for (int i = 0; i < foundDevices.getCount() - 1; i++) {
    auto device = foundDevices.getDevice(i);

    if (device.getName() == "DEV-BLE-PRP"){
      Serial.println("Device found!");

      auto client = BLEDevice::createClient();

      if (!client->connect(device.getAddress())) {
        Serial.println("Failed to connect!");
        break;
      }

      auto *pRemoteService = client->getService(serviceUUID);
      if (pRemoteService == nullptr) {
        Serial.println("Failed to retrieve using serviceUUID!");
        client->disconnect();
        break;
      }

      // Obtain a reference to the characteristic in the service of the remote BLE server.
      auto pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
      if (pRemoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(charUUID.toString().c_str());
        client->disconnect();
        break;
      }

      // Read the value of the characteristic.
      if(pRemoteCharacteristic->canRead()) {
        auto rawData = pRemoteCharacteristic->readValue();
        
        String hexString = "";  // String to accumulate the hex values

        for (const auto& byte : rawData) {
            hexString += String((uint8_t)byte, HEX);  // Convert byte to hex and append to hexString
            hexString += " ";  // Add space between hex values
        }

        Serial.print("The value retrieved is: ");
        Serial.println(hexString);

        auto current_speed = hexString.toInt();

        if (current_speed > 70) {
          smd.setRedValue(LED_BRIGHTNESS);
          delay(100);
          buzzer.playTooHighSpeed();
        } else {
          smd.setRedValue(0);
        }
      } else {
        Serial.println("Characteristic is not readable.");
      }
    }
  }

  pBLEScan->clearResults();  // delete results from BLEScan buffer to release memory

  delay(5000);
}
