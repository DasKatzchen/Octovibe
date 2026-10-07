#ifndef BLECOMMUNICATION_H
#define BLECOMMUNICATION_H
#include "ICommunication.h"
#include "../../config.h"

#include <NimBLEDevice.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#ifndef BLE_SERVICE_UUID
#define BLE_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#endif

#ifndef BLE_RX_CHARACTERISTIC_UUID
#define BLE_RX_CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#endif

#ifndef BLE_TX_CHARACTERISTIC_UUID
#define BLE_TX_CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#endif

#ifndef BLE_MAX_MESSAGE_LENGTH
#define BLE_MAX_MESSAGE_LENGTH 128
#endif

#ifndef BLE_RX_QUEUE_LENGTH
#define BLE_RX_QUEUE_LENGTH 32
#endif

struct BLEMessage {
    char data[BLE_MAX_MESSAGE_LENGTH];
};

class BLECommunication : public ICommunication {
private:
    //bool m_isOpen;
    NimBLEServer* pServer;
    QueueHandle_t rxQueue;
    
public:
    BLECommunication();

    bool isOpen() override;

    void start() override;
	
    void output(char* data) override;

    bool readData(char* input, size_t size) override;
};

#endif