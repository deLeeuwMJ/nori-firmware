#pragma once

#include "functional"
#include "NimBLEDevice.h"
#include "../common/events.hpp"

#define BLE_SCAN_INTERVAL_MS                2500
#define BLE_SCAN_WINDOW_MS                  1250
#define BLE_VLINK_SERVICE_UUID              "e7810a71-73ae-499d-8c15-faa9aef0c3f2"
#define BLE_CONTROL_CHARACTERISTIC_UUID     "bef8d6c9-9c21-4c9e-b632-bd58c1009f9f"
#define BLE_PERIPHERAL_DEVICE_NAME          "IOS-Vlink"
#define BLE_CENTRAL_DEVICE_NAME             "NORI-DISPLAY"
#define LOG_TAG_BLE                         "BLE"

namespace core
{
    enum BleState
    {
        IDLE,
        SCAN,
        CONNECT,
        CONNECTED,
        DISCONNECTED,
        ERROR,
    };

    class Bluetooth : public NimBLEScanCallbacks
    {
        public:
            void setup(std::function<void(Events::CarEventData)> dataCallback);
        private:
            NimBLEClient *client = nullptr;
            NimBLERemoteService *service = nullptr;
            NimBLERemoteCharacteristic *characteristic = nullptr;

            static void notifyCallback(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

            void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override;
           
            void connectToDevice();
            void pollCarEventData(Events::CarEvent event);
    };
}