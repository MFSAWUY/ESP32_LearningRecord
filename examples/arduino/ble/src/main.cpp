#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <atomic>
constexpr char SERVICE[]="d7610001-8cf1-4a1a-b8db-2f52fbd08b7b";
constexpr char VALUE[]="d7610002-8cf1-4a1a-b8db-2f52fbd08b7b";
BLECharacteristic *value=nullptr;
std::atomic<bool> connected{false},restart{false};
class Events:public BLEServerCallbacks{
  void onConnect(BLEServer*)override{connected.store(true);}
  void onDisconnect(BLEServer*)override{connected.store(false);restart.store(true);}
};
class Writes:public BLECharacteristicCallbacks{
  void onWrite(BLECharacteristic *c)override{
    String received=c->getValue();
    if(received.length()<=32)Serial.printf("BLE write=%s\n",received.c_str());
    else Serial.println("BLE value too long");
  }
};
void setup(){
  Serial.begin(115200);BLEDevice::init("ESP32-Handbook");
  auto server=BLEDevice::createServer();server->setCallbacks(new Events);
  auto service=server->createService(SERVICE);
  value=service->createCharacteristic(VALUE,BLECharacteristic::PROPERTY_READ|BLECharacteristic::PROPERTY_WRITE|BLECharacteristic::PROPERTY_NOTIFY);
  value->setCallbacks(new Writes);value->addDescriptor(new BLE2902);value->setValue("ready");service->start();
  auto adv=BLEDevice::getAdvertising();adv->addServiceUUID(SERVICE);adv->start();
}
void loop(){
  static uint32_t last=0;if(restart.exchange(false))BLEDevice::startAdvertising();
  if(connected.load()&&millis()-last>=1000){last=millis();char text[20];snprintf(text,sizeof(text),"uptime=%lu",millis()/1000UL);value->setValue(text);value->notify();}
  delay(10);
}
