#include <stdio.h>
#include "esp_random.h"
#include <NimBLEDevice.h>

static NimBLEServer* pServer;

static constexpr char* OBD2_SERVICE_UUID = "52d764ea-122d-4d01-8326-53e00a6ca36d";
static constexpr char* SPEED_CHAR_UUID = "a1247688-4bf1-40ec-bd6a-91724982e094";

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        printf("Client address: %s\n", connInfo.getAddress().toString().c_str());

        /**
         *  We can use the connection handle here to ask for different connection parameters.
         *  Args: connection handle, min connection interval, max connection interval
         *  latency, supervision timeout.
         *  Units; Min/Max Intervals: 1.25 millisecond increments.
         *  Latency: number of intervals allowed to skip.
         *  Timeout: 10 millisecond increments.
         */
        pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 0, 180);
    }

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        printf("Client disconnected - start advertising\n");
        NimBLEDevice::startAdvertising();
    }

    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
        printf("MTU updated: %u for connection ID: %u\n", MTU, connInfo.getConnHandle());
    }

    /********************* Security handled here *********************/
    uint32_t onPassKeyDisplay() override {
        printf("Server Passkey Display\n");
        /**
         * This should return a random 6 digit number for security
         *  or make your own static passkey as done here.
         */
        return 123456;
    }

    void onConfirmPassKey(NimBLEConnInfo& connInfo, uint32_t pass_key) override {
        printf("The passkey YES/NO number: %" PRIu32 "\n", pass_key);
        /** Inject false if passkeys don't match. */
        NimBLEDevice::injectConfirmPasskey(connInfo, true);
    }

    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
        /** Check that encryption was successful, if not we disconnect the client */
        if (!connInfo.isEncrypted()) {
            NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
            printf("Encrypt connection failed - disconnecting client\n");
            return;
        }

        printf("Secured connection to: %s\n", connInfo.getAddress().toString().c_str());
    }
} serverCallbacks;

extern "C" void app_main(void) {
    NimBLEDevice::init("Nori-Simulator");
    NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks);
    
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