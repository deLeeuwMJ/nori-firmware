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
    render->UpdateIntValue();
}

#include "NimBLEDevice.h"

static const NimBLEAdvertisedDevice* advDevice;
static bool                          doConnect  = false;
static uint32_t                      scanTimeMs = 5000; /** scan time in milliseconds, 0 = scan forever */
static NimBLEAddress peripheralAddress= NimBLEAddress("5C:37:BC:67:22:6E", BLE_ADDR_RANDOM);

class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        if (advertisedDevice->getAddress() == peripheralAddress) {
            printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());

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
    std::string str  = (isNotify == true) ? "Notification" : "Indication";
    str             += " from ";
    str             += pRemoteCharacteristic->getClient()->getPeerAddress().toString();
    str             += ": Service = " + pRemoteCharacteristic->getRemoteService()->getUUID().toString();
    str             += ", Characteristic = " + pRemoteCharacteristic->getUUID().toString();
    str             += ", Value = " + std::string((char*)pData, length);
    printf("%s\n", str.c_str());
}

bool connectToServer() {
    NimBLEClient* pClient = nullptr;

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
        pClient->setConnectionParams(12, 12, 0, 150);
        pClient->setConnectTimeout(5 * 1000);

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

    /** Now we can read/write/subscribe the characteristics of the services we are interested in */
    NimBLERemoteService*        pSvc = nullptr;
    NimBLERemoteCharacteristic* pChr = nullptr;

    pSvc = pClient->getService("0000180d-0000-1000-8000-00805f9b34fb"); // Heart Rate Service
    if (pSvc) {
        pChr = pSvc->getCharacteristic("00002a37-0000-1000-8000-00805f9b34fb"); // Heart Rate Notify
    }

    if (pChr) {
        if (pChr->canNotify()) {
            if (!pChr->subscribe(true, notifyCB)) {
                pClient->disconnect();
                return false;
            }
        }
    } else {
        printf("Heart rate service not found.\n");
    }

    printf("Done with this device!\n");

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

    printf("Scanning for peripherals\n");

    /** Loop here until we find a device we want to connect to */
    for (;;) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (doConnect) {
            doConnect = false;

            if (connectToServer()) {
                printf("Success! we should now be getting notifications, scanning for more!\n");
            } else {
                printf("Failed to connect, starting scan\n");
            }
        }
    }
}

// FreeRTOS is depended on calling a c app_main, hence extern "C"
extern "C" void app_main(void) {
    ESP_ERROR_CHECK(core::init_spi_bus());
    ESP_ERROR_CHECK(core::init_i2c_bus());

    // touch = new Touch();
    // touch->setup(onTouch);

    // touchDisplay = new TouchDisplay();
    // touchDisplay->init();

    // render = new Render();
    // render->setup(*touchDisplay);

    setupBluetooth();
}