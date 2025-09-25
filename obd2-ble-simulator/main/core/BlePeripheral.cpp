#include "BlePeripheral.hpp"
#include "esp_log.h"
#include "../common/events.hpp"
#include <stdio.h>
#include "esp_random.h"
#include <sstream>

namespace core
{
    void BluetoothPeripheral::setup()
    {
        NimBLEDevice::init(PERIPHERAL_NAME);
        NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);

        server = NimBLEDevice::createServer();
        server->setCallbacks(this);

        NimBLEService*        pObd2Service = server->createService(VLINK_SERVICE_UUID);
        NimBLECharacteristic* pControlCharacteristic = pObd2Service->createCharacteristic(CNTRL_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE);

        pControlCharacteristic->setCallbacks(this);

        pObd2Service->start();

        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        pAdvertising->setName(ADVERTISING_NAME);
        pAdvertising->addServiceUUID(pObd2Service->getUUID());

        pAdvertising->enableScanResponse(true);
        pAdvertising->start();

        ESP_LOGI(LOG_TAG_BLE_PERIPHERAL, "Advertising Started\n");

        for (;;) {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

    void BluetoothPeripheral::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo)
    {
        ESP_LOGI(LOG_TAG_BLE_PERIPHERAL, "Client connected: %s\n", connInfo.getAddress().toString().c_str());
    }

    void BluetoothPeripheral::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason)
    {
        ESP_LOGI(LOG_TAG_BLE_PERIPHERAL, "Client disconnected, restart advertising\n");
        NimBLEDevice::startAdvertising();
    }

    /*
        To act as a digital twin, we will simulate the response sent by a VLink device
        when it receives a command from an app like OBDLink or OBDLinkX.
        This starts with repeating the sent command, then sending the response, and finishing with ending characters.
    */
    void BluetoothPeripheral::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)
    {
        echoMessage(pCharacteristic);
        writeResultOfRequest(pCharacteristic);
        sendEndOfMessage(pCharacteristic);
    }

    void BluetoothPeripheral::echoMessage(NimBLECharacteristic* pCharacteristic)
    {
        std::string value = pCharacteristic->getValue().c_str();
        if (value.size() > 0) {
            ESP_LOGI(LOG_TAG_BLE_PERIPHERAL, "%s : onWrite(), value: %s\n",pCharacteristic->getUUID().toString().c_str(), value.c_str());
            pCharacteristic->notify(value);
        }
    }

    void BluetoothPeripheral::writeResultOfRequest(NimBLECharacteristic* pCharacteristic)
    {
        std::string rawValue = pCharacteristic->getValue().c_str();
        if (rawValue.size() > 0) {
            std::string commandStr = rawValue.substr(2,3);
           ESP_LOGI(LOG_TAG_BLE_PERIPHERAL, "Command: %s\n", commandStr.c_str());

            if (strcmp(commandStr.c_str(), "0D")) {
                uint32_t randomNumber = esp_random() % 130;

                std::stringstream randomNumberHex;
                randomNumberHex << std::hex << randomNumber;

                std::string responseStr = "41 0D "+ randomNumberHex.str() + " \r";
                pCharacteristic->notify(responseStr);
            }
        }
    }

    void BluetoothPeripheral::sendEndOfMessage(NimBLECharacteristic* pCharacteristic)
    {
        pCharacteristic->notify(std::string(END_OF_MESSAGE));
    }
}
