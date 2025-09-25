#pragma once

#include <NimBLEDevice.h>

#define LOG_TAG_BLE_PERIPHERAL      "BLE_PERIPHERAL"

#define PERIPHERAL_NAME             "Vlink-Simulator"
#define ADVERTISING_NAME            "IOS-Vlink"
#define VLINK_SERVICE_UUID          "e7810a71-73ae-499d-8c15-faa9aef0c3f2"
#define CNTRL_CHAR_UUID             "bef8d6c9-9c21-4c9e-b632-bd58c1009f9f"

#define END_OF_MESSAGE              "\r>"

namespace core
{
    class BluetoothPeripheral : public NimBLEServerCallbacks, NimBLECharacteristicCallbacks
    {
        public:
        
            void setup();
        private:
            NimBLEServer* server = nullptr;
            
            void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
            void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
            void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;

            void echoMessage(NimBLECharacteristic* pCharacteristic);
            void writeResultOfRequest(NimBLECharacteristic* pCharacteristic);
            void sendEndOfMessage(NimBLECharacteristic* pCharacteristic);
    };
}