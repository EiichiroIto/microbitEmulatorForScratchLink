#include <M5Unified.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include "scratchlink.h"

// Scratch Link for micro:bit
//    service: 0xf005,
//    rxChar: '5261da01-fa7e-42ab-850b-7c80220097cc',
//    txChar: '5261da02-fa7e-42ab-850b-7c80220097cc'
#define SERVICE_UUID                "0000f005-0000-1000-8000-00805F9B34FB"
#define READ_CHARACTERISTIC_UUID    "5261da01-fa7e-42ab-850b-7c80220097cc"
#define WRITE_CHARACTERISTIC_UUID   "5261da02-fa7e-42ab-850b-7c80220097cc"

BLECharacteristic* CurrentCharacteristic = NULL;
BLECharacteristic *pReadCharacteristic = NULL;
BLECharacteristic *pWriteCharacteristic = NULL;

bool NotifyEnabled = false;

static void scratchlink_startadvertising()
{
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
}

static void scratchlink_send(byte *data)
{
  if (!CurrentCharacteristic) {
    return;
  }
  CurrentCharacteristic->setValue(data, 10);
  CurrentCharacteristic->notify();
  Serial.println("scratchlink_send");
}

class MyDescriptorCallbacks : public BLEDescriptorCallbacks {
private:
  const char* characteristicName;

public:
  MyDescriptorCallbacks(const char* name) {
      characteristicName = name;
  }

  void onWrite(BLEDescriptor* pDescriptor) override {
    Serial.println("===== Descriptor Write =====");
    Serial.print("Characteristic: ");
    Serial.println(characteristicName);
    Serial.print("Descriptor UUID: ");
    Serial.println(pDescriptor->getUUID().toString().c_str());

    const uint8_t* value = pDescriptor->getValue();
    size_t length =  pDescriptor->getLength();

    Serial.print("Value HEX: ");

    for (size_t i = 0; i < length; i++) {
      Serial.printf("%02X ", (uint8_t)value[i]);
    }

    Serial.println();

    if (length >= 2) {
      uint16_t v = (uint8_t)value[0] | ((uint8_t)value[1] << 8);
      Serial.printf("Subscription value: 0x%04X\n", v);
      if (v == 0x0001) {
        NotifyEnabled = true;
        Serial.println(">>> NOTIFY ENABLED");
      } else if (v == 0x0000) {
        NotifyEnabled = false;
        Serial.println(">>> NOTIFY DISABLED");
      } else if (v == 0x0002) {
        Serial.println(">>> INDICATE ENABLED");
      }
    }
  }
};

class MyCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer){
    Serial.println("ScratchLink: Connected\n");
    CurrentCharacteristic = NULL;
  }
  void onDisconnect(BLEServer* pServer){
    Serial.println("ScratchLink: Disconnected\n");
    scratchlink_startadvertising();
    CurrentCharacteristic = NULL;
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic* pCharacteristic){
    Serial.println("onWrite");
    Serial.println(pCharacteristic->toString().c_str());
    int num = pCharacteristic->getLength();
    byte *payload = pCharacteristic->getData();
    byte command = *payload++;
    scratchlink_callback(command, payload, num - 1);
  }
  void onRead(BLECharacteristic* pCharacteristic){
    Serial.println("onRead");
    //Serial.println(pCharacteristic->toString().c_str());

    std::string value = pCharacteristic->getValue();
    Serial.print("length=");
    Serial.println(value.length());
    Serial.print("HEX: ");
    for (size_t i = 0; i < value.length(); i++) {
        Serial.printf("%02X ", (uint8_t)value[i]);
    }
    Serial.println();
    Serial.print("TEXT: ");
    Serial.println(value.c_str());

    CurrentCharacteristic = pCharacteristic;
    u_int8_t data[10];
    memset(data, 0, sizeof data);
    scratchlink_send(data);
  }
  void onNotify(BLECharacteristic* pCharacteristic){
    //Serial.println("onNotify");
  }
};

void init_scratchlink(const char *devicename)
{
  BLEDevice::init(devicename);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  pServer->setCallbacks(new MyCallbacks());

  pReadCharacteristic = pService->createCharacteristic(
    READ_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pWriteCharacteristic = pService->createCharacteristic(
    WRITE_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  MyCharacteristicCallbacks *callback = new MyCharacteristicCallbacks;
  pReadCharacteristic->setCallbacks(callback);
  pWriteCharacteristic->setCallbacks(callback);

  BLE2902* readCCCD = new BLE2902();
  readCCCD->setCallbacks(
      new MyDescriptorCallbacks("Read")
  );
  pReadCharacteristic->addDescriptor(readCCCD);

  BLE2902* writeCCCD = new BLE2902();
  writeCCCD->setCallbacks(
      new MyDescriptorCallbacks("Write")
  );
  pWriteCharacteristic->addDescriptor(writeCCCD);

  pService->start();

  scratchlink_startadvertising();
}

// scratchlink_update
// data frame
//     0           1          2           3          4         5         6        7        8        9
// [AccX High] [AccX Low] [AccY High] [AccY Low] [ButtonA] [ButtonB] [Touch0] [Touch1] [Touch2] [Gesture]
void scratchlink_update(uint16_t accx, uint16_t accy, bool buttonA, bool buttonB, bool touch0, bool touch1, bool touch2, int gesture)
{
  if (!NotifyEnabled) {
    return;
  }
  u_int8_t data[10];
  data[0] = (accx >> 8) % 256;
  data[1] = accx % 256;
  data[2] = (accy >> 8) % 256;
  data[3] = accy % 256;
  data[4] = buttonA ? 1 : 0;
  data[5] = buttonB ? 1 : 0;
  data[6] = touch0 ? 1 : 0;
  data[7] = touch1 ? 1 : 0;
  data[8] = touch2 ? 1 : 0;
  data[9] = gesture % 256;
  Serial.print(data[4]);
  scratchlink_send(data);
}

bool scratchlink_isconnecting()
{
  return CurrentCharacteristic != NULL;
}


