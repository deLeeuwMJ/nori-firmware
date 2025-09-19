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
static constexpr char* OBD2_SERVICE_UUID = "52d764ea-122d-4d01-8326-53e00a6ca36d";
static constexpr char* SPEED_CHAR_UUID = "a1247688-4bf1-40ec-bd6a-91724982e094";

class ClientCallbacks : public NimBLEClientCallbacks {
    void onConnect(NimBLEClient* pClient) override { printf("Connected\n"); }

    void onDisconnect(NimBLEClient* pClient, int reason) override {
        printf("%s Disconnected, reason = %d - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
        NimBLEDevice::getScan()->start(scanTimeMs, false, true);
    }

    /********************* Security handled here *********************/
    void onPassKeyEntry(NimBLEConnInfo& connInfo) override {
        printf("Server Passkey Entry\n");
        /**
         * This should prompt the user to enter the passkey displayed
         * on the peer device.
         */
        NimBLEDevice::injectPassKey(connInfo, 123456);
    }

    void onConfirmPasskey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
        printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }

    /** Pairing process complete, we can check the results in connInfo */
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        if (!connInfo.isEncrypted()) {
            printf("Encrypt connection failed - disconnecting\n");
            /** Find the client with the connection handle provided in connInfo */
            NimBLEDevice::getClientByHandle(connInfo.getConnHandle())->disconnect();
            return;
        }
    }
} clientCallbacks;


class ScanCallbacks : public NimBLEScanCallbacks {
    void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
        printf("Advertised Device found: %s\n", advertisedDevice->toString().c_str());

        if (advertisedDevice->getAddress() == peripheralAddress) {
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
    render->UpdateValue(*pData);
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
        pClient->setClientCallbacks(&clientCallbacks, false);

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

    pSvc = pClient->getService(OBD2_SERVICE_UUID);
    if (pSvc) {
        pChr = pSvc->getCharacteristic(SPEED_CHAR_UUID);
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

    for (;;) {
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if (doConnect) {
            doConnect = false;

            if (connectToServer()) {
                printf("Successfully subscribed to service!\n");
            } 
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