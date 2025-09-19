#include <stdio.h>
#include "esp_random.h"
#include <NimBLEDevice.h>

static NimBLEServer* pServer;

static constexpr char* OBD2_SERVICE_UUID = "52d764ea-122d-4d01-8326-53e00a6ca36d";
static constexpr char* SPEED_CHAR_UUID = "a1247688-4bf1-40ec-bd6a-91724982e094";

extern "C" void app_main(void) {
    NimBLEDevice::init("Nori-Simulator");
    NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);

    pServer = NimBLEDevice::createServer();

    NimBLEService*        pObd2Service = pServer->createService(OBD2_SERVICE_UUID);
    NimBLECharacteristic* pSpeedCharacteristic = pObd2Service->createCharacteristic(SPEED_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY);
    
    pSpeedCharacteristic->setValue(0x00);
    pObd2Service->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->setName("Nori-Simulator-Obd2");
    pAdvertising->addServiceUUID(pObd2Service->getUUID());

    pAdvertising->enableScanResponse(true);
    pAdvertising->start();

    printf("Advertising Started\n");

    for (;;) {
        if (pServer->getConnectedCount()) {
            NimBLEService* pSvc = pServer->getServiceByUUID(OBD2_SERVICE_UUID);
            if (pSvc) {
                NimBLECharacteristic* pChr = pSvc->getCharacteristic(SPEED_CHAR_UUID);
                if (pChr) {
                    uint32_t randomNumber = esp_random() % 150;
                    pChr->notify(randomNumber);
                }
            }
        }
        vTaskDelay(2500 / portTICK_PERIOD_MS);
    }
}