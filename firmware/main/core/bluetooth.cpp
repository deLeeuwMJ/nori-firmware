#include "bluetooth.hpp"

namespace core
{
    static BleState bleState = BleState::IDLE;
    static std::function<void(Events::CarEventData)> dataCallback = nullptr;

    const NimBLEAdvertisedDevice* advDevice = nullptr;

    void Bluetooth::notifyCallback(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)
    {
        // Temporary skipping other messages
        std::string rawValue = std::string((char*)pData, length);
        if (strcmp(rawValue.c_str(),"010C\r") || strcmp(rawValue.c_str(),"\r>"))
            return;

        Events::CarEventResponse response = Events::parseRawData(pData);

        if (Events::isSuccesfull(response))
            dataCallback(Events::retrieveData(response));
    }

    void Bluetooth::onResult(const NimBLEAdvertisedDevice* advertisedDevice) {
        ESP_LOGI(LOG_TAG_BLE, "Device found: %s\n", advertisedDevice->toString().c_str());

        if (advertisedDevice->getName() == BLE_PERIPHERAL_DEVICE_NAME) {
            advDevice = advertisedDevice;
            bleState = BleState::CONNECT;
        }
    }

    void Bluetooth::pollCarEventData(Events::CarEvent event)
    {
        switch(event)
        {
            case Events::CarEvent::VEHICLE_SPEED:
                characteristic->writeValue("010D\r");
                break;
            default:
                ESP_LOGI(LOG_TAG_BLE, "Unkown event requested");
        }

    }

    void Bluetooth::setup(std::function<void(Events::CarEventData)> dataCallback)
    {
        dataCallback = dataCallback;

        NimBLEDevice::init(BLE_CENTRAL_DEVICE_NAME);
        NimBLEScan* pScan = NimBLEDevice::getScan();
        pScan->setScanCallbacks(this, false);
        pScan->setInterval(BLE_SCAN_INTERVAL_MS);
        pScan->setWindow(BLE_SCAN_WINDOW_MS);
        pScan->setActiveScan(false);

        for (;;) {
            switch (bleState)
            {
                case core::BleState::IDLE:
                    pScan->start(0, false);
                    bleState = BleState::SCAN;
                    break;
                case core::BleState::CONNECT:
                    pScan->stop();
                    connectToDevice();
                    break;
                case core::BleState::CONNECTED:
                    pollCarEventData(Events::CarEvent::VEHICLE_SPEED);
                    break; 
                case core::BleState::DISCONNECTED:
                    bleState = BleState::IDLE;
                    break;
                default:
                    // BleState::SCAN is intentionally ignored
                    break;
            }

            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    };

    void Bluetooth::connectToDevice()
    {
        /** Check if we have a client we should reuse first **/
        if (NimBLEDevice::getCreatedClientCount()) {
            /**
             *  Special case when we already know this device, we send false as the
             *  second argument in connect() to prevent refreshing the service database.
             *  This saves considerable time and power.
             */
            client = NimBLEDevice::getClientByPeerAddress(advDevice->getAddress());

            if (client) {
                if (!client->connect(advDevice, false))
                {
                    ESP_LOGI(LOG_TAG_BLE, "Reconnect failed");
                    bleState = BleState::DISCONNECTED;
                    return;
                }
                   
                ESP_LOGI(LOG_TAG_BLE, "Reconnected");
            } else {
                /**
                 *  We don't already have a client that knows this device,
                 *  check for a client that is disconnected that we can use.
                 */
                client = NimBLEDevice::getDisconnectedClient();
            }
        }

        /** No client to reuse? Create a new one. */
        if (!client) {
            if (NimBLEDevice::getCreatedClientCount() >= MYNEWT_VAL(BLE_MAX_CONNECTIONS)) {
                ESP_LOGI(LOG_TAG_BLE, "Max clients reached - no more connections available");
                bleState = BleState::ERROR;
                return;
            }

            client = NimBLEDevice::createClient();
            if (!client->connect(advDevice)) {
                NimBLEDevice::deleteClient(client);
                ESP_LOGI(LOG_TAG_BLE, "Failed to connect, deleted client");
                bleState = BleState::DISCONNECTED;
                return;
            }
        }

        if (!client->isConnected()) {
            if (!client->connect(advDevice)) {
                ESP_LOGI(LOG_TAG_BLE, "Failed to connect");
                bleState = BleState::DISCONNECTED;
                return;
            }
        }

        ESP_LOGI(LOG_TAG_BLE, "Connected to: %s RSSI: %d\n", client->getPeerAddress().toString().c_str(), client->getRssi());

        service = client->getService(BLE_VLINK_SERVICE_UUID);
        if (service) 
            characteristic = service->getCharacteristic(BLE_CONTROL_CHARACTERISTIC_UUID);
        
        if (characteristic) {
            if (characteristic->canNotify()) {
                if (!characteristic->subscribe(true, notifyCallback)) {
                    client->disconnect();
                    ESP_LOGI(LOG_TAG_BLE, "Failed to connect");
                    bleState = BleState::DISCONNECTED;
                    return;
                } else {
                    bleState = BleState::CONNECTED;
                }
            }
        } else {
            printf("Service not found.\n");
        }
    }

}