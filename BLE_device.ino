#include "BLEDevice.h"
#include "Math.h"
#include "EEPROM.h"
#include "BluetoothSerial.h"

#define RXD2 16
#define TXD2 17

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

static BLEAddress *pServerAddress;
BLEScan* pBLEScan;
BLEClient*  pClient;

String beaconAddress[] = {"c6:2d:9d:ae:08:81", "fe:af:87:0f:32:db", "f3:a6:b7:6e:76:54", "e3:7d:9c:7e:f5:7d", "d4:dd:24:b1:16:4f"};
String message = "";
String msgToSend;
int beaconLength = 0;
String beaconStatus[20];
char msg[512];
String Status = "";
bool detected = false;
int state = 0, count = 0;
long endTime1 = 0.0;
long endTime2 = 0.0;
int i;
int counter = 0;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice Device) {
      Serial.print(state);
      pServerAddress = new BLEAddress(Device.getAddress());
      if (strcmp(pServerAddress->toString().c_str(), beaconAddress[state].c_str()) == 0) {
        detected = true;
      }
      if (detected) {
        int rssi = Device.getRSSI();
        if (rssi > -50) {
          Status = "I";
        } else if (rssi > -80 && rssi <= -50) {
          Status = "N";
        }
        else if (rssi <= -80) {
          Status = "F";
        } else {
          Status = "U";
        }
        Device.getScan()->stop();
      } else {
        Status = "U";
      }
    }
};


void Working() {
  Serial.println(state);
  beaconStatus[state] = "";
  detected = false;
  BLEScanResults scanResults = pBLEScan->start(5);
  beaconStatus[state] = Status;
  if (state < beaconLength) {
    state = state + 1;
  }
  if (state == beaconLength) {
    state = 0;
  }
}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  SerialBT.begin("ESP32test");
  beaconLength = sizeof(beaconAddress) / 12;
  Serial.println(beaconLength);
  BLEDevice::init("");
  pClient  = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("Init BLE Done !");
}

void makeString() {
  message = "";
  i = 0;
  while (i < beaconLength) {
    Serial.print("i value : ");
    Serial.print(i);
    Serial.print(" address : ");
    Serial.print(beaconAddress[i]);
    Serial.print(" status : ");
    Serial.println(beaconStatus[i]);
    message = String(message) + String(beaconAddress[i]) + String("-") + String(beaconStatus[i]) + String("|");
    i++;
  }
}

void sendStrings() {
  snprintf (msg, 512, "#SHCOOL|%s,%d,", msgToSend.c_str(), counter );
  Serial2.write(msg);
  Serial.println(msg);
  if (counter < 21) {
    counter = counter + 1;
  } else {
    counter = 0;
  }

}

void loop() {
  Working();
  delay(100);
  long nowtime1 = millis();
  if (nowtime1 - endTime1 > 60000) {
    endTime1 = nowtime1;
    sendStrings();
  }
  if (state == 0) {
    makeString();
    msgToSend = message;
    memset(beaconStatus, 0, sizeof beaconStatus);
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
}
