#include <stdio.h>
#include "esp_err.h"
#include "config.hpp"
#include "display.hpp"
#include "render.hpp"
#include "touch.hpp"
#include "core.hpp"

Touch* touch = nullptr;
TouchDisplay* touchDisplay = nullptr;
Render* render = nullptr;

void onTouch(uint16_t x, uint16_t y) {
    ESP_LOGI("MAIN", "Touch handled in main! Coordinates: (%d, %d)", x, y);
}

#include "NimBLEDevice.h"

static const NimBLEAdvertisedDevice* advDevice;
static bool                          doConnect  = false;
static uint32_t                      scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */

static NimBLEAddress peripheralAddress= NimBLEAddress("3c:71:bf:fd:83:56", BLE_ADDR_PUBLIC);
static constexpr char* VLINK_SERVICE_UUID = "e7810a71-73ae-499d-8c15-faa9aef0c3f2";
static constexpr char* CNTRL_CHAR_UUID = "bef8d6c9-9c21-4c9e-b632-bd58c1009f9f";

class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());

        if (advertisedDevice->getName() == "IOS-Vlink") {
        // if (advertisedDevice->getAddress() == peripheralAddress) {
            /** stop scan before connecting */
            NimBLEDevice::getScan()->stop();

            /** Save the device reference in a global for the client to use*/
            advDevice = advertisedDevice;

            /** Ready to connect now */
            doConnect = true;
        }
    }

    void onScanEnd(const NimBLEScanResults& results, int reason) override {
        printf("Scan Ended, reason: %d, device count: %d; Restarting scan\n", reason, results.getCount());
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }
} scanCallbacks;

void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    std::string rawValue = std::string((char*)pData, length);
    if (rawValue.starts_with("41 0D"))
    {
        std::string retrievedSpeed = rawValue.substr(6,2); //41 0D 33 \r
        printf("Retrieved speed HEX: %s\n", retrievedSpeed.c_str());
        uint8_t decimalValue = std::stoi(retrievedSpeed, nullptr, 16);

        printf("Retrieved speed DEC: %u\n", decimalValue);
        render->UpdateValue(decimalValue);
    }
}

NimBLEClient* pClient = nullptr;
NimBLERemoteService*        pSvc = nullptr;
NimBLERemoteCharacteristic* pChr = nullptr;

bool connectToServer() {
    /** Check if we have a client we should reuse first **/
    if (NimBLEDevice::getCreatedClientCount()) {
        /**
         *  Special case when we already know this device, we send false as the
         *  second argument in connect() to prevent refreshing the service database.
         *  This saves considerable time and power.
         */
        pClient = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());
        if (pClient) {
            if (!pClient->connect(advDevice, false)) {
                printf("Reconnect failed\n");
                return false;
            }
            printf("Reconnected client\n");
        } else {
            /**
             *  We don't already have a client that knows this device,
             *  check for a client that is disconnected that we can use.
             */
            pClient = NimBLEDevice::getDisconnectedClient();
        }
    }

    /** No client to reuse? Create a new one. */
    if (!pClient) {
        if (NimBLEDevice::getCreatedClientCount() >= MYNEWT_VAL(BLE_MAX_CONNECTIONS)) {
            printf("Max clients reached - no more connections available\n");
            return false;
        }

        pClient = NimBLEDevice::createClient();

        if (!pClient->connect(advDevice)) {
            NimBLEDevice::deleteClient(pClient);
            printf("Failed to connect, deleted client\n");
            return false;
        }
    }

    if (!pClient->isConnected()) {
        if (!pClient->connect(advDevice)) {
            printf("Failed to connect\n");
            return false;
        }
    }

    printf("Connected to: %s RSSI: %d\n", pClient->getPeerAddress().toString().c_str(), pClient->getRssi());

    pSvc = pClient->getService(VLINK_SERVICE_UUID);
    if (pSvc) {
        pChr = pSvc->getCharacteristic(CNTRL_CHAR_UUID);
    }

    if (pChr) {
        if (pChr->canNotify()) {
            if (!pChr->subscribe(true, notifyCB)) {
                pClient->disconnect();
                return false;
            }
        }
    } else {
        printf("Service not found.\n");
    }

    return true;
}

void setupBluetooth()
{
    NimBLEDevice::init("Nori-Display");
    NimBLEScan* pScan = NimBLEDevice::getScan();

    pScan->setScanCallbacks(&scanCallbacks, false);
    pScan->setInterval(5000);
    pScan->setWindow(5000);
    pScan->setActiveScan(true);
    pScan->start(scanTimeMs);

    bool subscribed = false;

    for (;;) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        if (doConnect) {
            doConnect = false;

            if (connectToServer()) {
                printf("Successfully subscribed!\n");
                subscribed = true;
            } 
        }

        if (subscribed)
        {
            pSvc = pClient->getService(VLINK_SERVICE_UUID);
            pChr = pSvc->getCharacteristic(CNTRL_CHAR_UUID);
            pChr->writeValue("010D\r");
        }
    }
}

// FreeRTOS is depended on calling a c app_main, hence extern "C"
extern "C" void app_main(void) {
    ESP_ERROR_CHECK(core::init_spi_bus());
    ESP_ERROR_CHECK(core::init_i2c_bus());

    touch = new Touch();
    touch->setup(onTouch);

    touchDisplay = new TouchDisplay();
    touchDisplay->init();

    render = new Render();
    render->setup(*touchDisplay);

    setupBluetooth();
}