#include <EEPROM.h>

#include "BLEDevice.h"
#include "Math.h"
#include <WiFi.h>
#include "BluetoothSerial.h"

#define RXD2 16
#define TXD2 17

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial ESP32BT;

static BLEAddress *pServerAddress;
BLEScan* pBLEScan;
BLEClient*  pClient;

String beaconAddress[12];
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
char BTmsg;
int EEPROMaddress[12];

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice Device) {
      Serial.print(state);
      pServerAddress = new BLEAddress(Device.getAddress());
      if (strcmp(pServerAddress->toString().c_str(), beaconAddress[state].c_str()) == 0) {
        detected = true;
      }
      if (detected) {
        int rssi = Device.getRSSI();
        if (rssi < -95) {
          Status = "U";
        } else {
          Status = "N";
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

void saveMacAddress(String msg) {
  Serial.println(msg);
  String msgArr[3];
  int t = 0, r = 0;
  for (int c = 0; c < msg.length(); c++) {
    if (msg.charAt(c) == ',' || msg.charAt(c) == '#') {
      msgArr[t] = msg.substring(r, c);
      r = c + 1;
      t++;
    }
  }
  Serial.print("BT Serial : ");
  Serial.print(msgArr[0]);
  Serial.print(" : ");
  Serial.print(msgArr[1]);
  Serial.print(" : ");
  Serial.println(msgArr[2]);

  if (msgArr[0] == "mac" || msgArr[0] == "MAC") {
    int address = msgArr[1].toInt();
    String mac = msgArr[2];
    Serial.println("Save EEPROM");
    EEPROM.writeString(EEPROMaddress[address - 1], mac);
    EEPROM.commit();
  }
  getBeaconAddressFromEEPROM();
}

void getBeaconAddressFromEEPROM() {
  beaconLength = 0;
  for (int i = 0; i < 12; i++) {
    String address = EEPROM.readString(EEPROMaddress[i]);
    Serial.print("EEPROM : ");
    Serial.println(address);
    if (address.length() >= 10) {
      beaconAddress[beaconLength] = address;
      beaconLength++;
    }
  }
}


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  String BTName = "ESP_" + WiFi.macAddress();
  ESP32BT.begin(BTName);
  EEPROM.begin(512);
  BLEDevice::init("");
  pClient  = BLEDevice::createClient();
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("Init BLE Done !");

  Serial.println(WiFi.macAddress());

  EEPROMaddress[0] = 0;
  for (int count = 1; count < 12; count++) {
    EEPROMaddress[count] = EEPROMaddress[count - 1] + 18;
    Serial.println(EEPROMaddress[count]);
  }

  getBeaconAddressFromEEPROM();
}

void makeString() {
  message = "";
  i = 0;
  Serial.println("");
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
  while (ESP32BT.available()) {
    String msg = ESP32BT.readString();
    saveMacAddress(msg);
  }
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
}
