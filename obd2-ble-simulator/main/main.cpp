#include <stdio.h>
#include "esp_random.h"
#include <NimBLEDevice.h>
#include <sstream>
#include <iomanip>

static NimBLEServer* pServer;

static constexpr char* VLINK_SERVICE_UUID = "e7810a71-73ae-499d-8c15-faa9aef0c3f2";
static constexpr char* CNTRL_CHAR_UUID = "bef8d6c9-9c21-4c9e-b632-bd58c1009f9f";

class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
    
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        std::string rawValue = pCharacteristic->getValue().c_str();

        // Sent message to repeat request
        printf("%s : onWrite(), value: %s\n",pCharacteristic->getUUID().toString().c_str(), rawValue.c_str());
        pCharacteristic->notify(rawValue);

        // Process command and respond
        if (rawValue.size() > 0) {
            std::string commandStr = rawValue.substr(2,3);
            printf("Command: %s\n", commandStr.c_str());

            if (strcmp(commandStr.c_str(), "0D")) {
                uint32_t randomNumber = esp_random() % 130;

                std::stringstream randomNumberHex;
                randomNumberHex << std::hex << randomNumber;

                std::string responseStr = "41 0D "+ randomNumberHex.str() + " \r";
                pCharacteristic->notify(responseStr);
            }
        }

        // Notify with ending message
        std::string endStr = "\r>";
        pCharacteristic->notify(endStr);
    }
} chrCallbacks;

extern "C" void app_main(void) {
    NimBLEDevice::init("Vlink-Simulator");
    NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);

    pServer = NimBLEDevice::createServer();
    
    NimBLEService*        pObd2Service = pServer->createService(VLINK_SERVICE_UUID);
    NimBLECharacteristic* pControlCharacteristic = pObd2Service->createCharacteristic(CNTRL_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE);
    
    pControlCharacteristic->setValue(0x00);
    pControlCharacteristic->setCallbacks(&chrCallbacks);

    pObd2Service->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("IOS-Vlink");
    pAdvertising->addServiceUUID(pObd2Service->getUUID());

    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    printf("Advertising Started\n");

    for (;;) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}