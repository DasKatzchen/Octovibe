#include "BLECommunication.h"

BLECommunication::BLECommunication() : pServer(nullptr), rxQueue(nullptr) {
    //m_isOpen = false;
}

// Queues every write to the RX characteristic so that writes arriving faster than
// loop() polls (e.g. one write per motor from Buttplug) are not lost, and each
// message is handled exactly once.
class RxCallbacks: public NimBLECharacteristicCallbacks {
public:
    explicit RxCallbacks(QueueHandle_t queue) : queue(queue) {}

    // Requires NimBLE-Arduino 2.x
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        auto value = pCharacteristic->getValue();
        size_t len = value.length();
        if (len == 0) {
            return;
        }
        BLEMessage msg;
        if (len >= sizeof(msg.data)) {
            len = sizeof(msg.data) - 1; // truncate oversized writes
        }
        memcpy(msg.data, value.c_str(), len);
        msg.data[len] = '\0';
        xQueueSend(queue, &msg, 0); // never block the BLE host task; drop if full
    }

private:
    QueueHandle_t queue;
};

class ServerCallbacks: public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        #ifdef NEOPIXEL
        neopixelWrite(DEBUG_LED,0,RGB_BRIGHTNESS,0); // Green
        #endif
    };
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        #ifdef NEOPIXEL
        neopixelWrite(DEBUG_LED,0,0,RGB_BRIGHTNESS); // Blue
        #endif
    };
};

bool BLECommunication::isOpen() {
    return pServer->getConnectedCount();
}

void BLECommunication::start() {
    NimBLEDevice::init(BTSERIAL_DEVICE_NAME);

    rxQueue = xQueueCreate(BLE_RX_QUEUE_LENGTH, sizeof(BLEMessage));

    pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
	pServer->advertiseOnDisconnect(true);
    NimBLEService *pService = pServer->createService(BLE_SERVICE_UUID); //Main service
    NimBLECharacteristic *rxCharacteristic = pService->createCharacteristic(BLE_RX_CHARACTERISTIC_UUID,NIMBLE_PROPERTY::WRITE | WRITE_NR);
    rxCharacteristic->setCallbacks(new RxCallbacks(rxQueue));
    NimBLECharacteristic *txCharacteristic = pService->createCharacteristic(BLE_TX_CHARACTERISTIC_UUID,NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    pService->start();

    NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising(); // create advertising instance
    pAdvertising->addServiceUUID(pService->getUUID()); // tell advertising the UUID of our service
	pAdvertising->setName(BTSERIAL_DEVICE_NAME);
    pAdvertising->start(); // start advertising
      
    #ifdef BT_ECHO
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("The device started, now you can pair it with bluetooth!");
    #endif
    #ifdef NEOPIXEL
    neopixelWrite(DEBUG_LED,0,0,RGB_BRIGHTNESS); // Blue
    #endif
    //m_isOpen = true;
}

void BLECommunication::output(const char* data) {
    if(pServer->getConnectedCount()) {
        NimBLEService* pSvc = pServer->getServiceByUUID(BLE_SERVICE_UUID);
        if(pSvc) {
            NimBLECharacteristic* qChr = pSvc->getCharacteristic(BLE_TX_CHARACTERISTIC_UUID);
            if(qChr) {
                qChr->setValue(String(data));
                qChr->notify();
            }
        }
	}
    //else
    //vTaskDelay(1); //keep watchdog fed
    #ifdef BT_ECHO
    Serial.print(data);
    Serial.flush();
    #endif
}

bool BLECommunication::readData(char* input, size_t size) {
    if (input == nullptr || size == 0 || rxQueue == nullptr) {
        return false;
    }
    BLEMessage msg;
    if (xQueueReceive(rxQueue, &msg, 0) != pdTRUE) {
        return false;
    }
    strncpy(input, msg.data, size - 1);
    input[size - 1] = '\0';
    return true;
}
