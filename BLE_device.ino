#include "BLEDevice.h"
#include "Math.h"


static BLEAddress *pServerAddress;
BLEScan* pBLEScan;
BLEClient*  pClient;

String beaconAddress[] = {"b4:52:a9:03:a6:23", "57:da:7a:82:3e:32"};
String Status = "";
bool detected = false;
int state = 0;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice Device) {
      //      Serial.print("BLE Advertised Device found: ");
      //            Serial.println(Device.toString().c_str());
      pServerAddress = new BLEAddress(Device.getAddress());
      if (strcmp(pServerAddress->toString().c_str(), beaconAddress[state].c_str()) == 0) {
        detected = true;
      }

      if (detected) {
        Serial.print("Device detected ");
        Serial.print(Device.toString().c_str());
        Serial.print(" RSSI = ");
        Serial.println(Device.getRSSI());
        int rssi = Device.getRSSI();
        if (rssi > -50) {
          Status = "Immediately";
        } else if (rssi > -90 && rssi <= -50) {
          Status = "Near";
        }
        else {
          Status = "Far";
        }
        Device.getScan()->stop();
        delay(100);
        Serial.print("Status: ");
        Serial.println(Status);
      }
    }
};


void Working() {
  Serial.println(state);
  Serial.println("BLE Scan restarted.....");
  detected = false;
  BLEScanResults scanResults = pBLEScan->start(5);
  if (state == 0) {
    state = 1;
  } else {
    state = 0;
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  BLEDevice::init("");
  pClient  = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("Init BLE Done !");
}

void loop() {
  // put your main code here, to run repeatedly:
  Working();
  delay(5000);
}
