#include "credentials.h"
// -------------------------------- INPUTS AND OUTPUTS STUFF --------------------------------
#include <SPI.h>
#include <Wire.h>
#define IR_PIN 4 // D2


// -------------------------------- WIFI AND FIREBASE STUFF --------------------------------
#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#include <NTPClient.h>
#include <WiFiUdp.h>
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);

unsigned int counter = 0;

bool prevColorWasWhite = true;
bool iHaveSeenBlackOnceBefore = false;
unsigned long lastEnteredBlackTime = 0;

unsigned long deltaT = 0;
float rpm = 0.0;
float rpmToSend = 0.0;

unsigned long lastSent = 0;
int sendInterval = 5000;

FirebaseData fbdo;

// -------------------------------- ARDUINO SETUP --------------------------------
void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  for (int k = 0; k < ((sizeof(SSIDs)) / (sizeof(char *))); k++) {
    wifiMulti.addAP(SSIDs[k], PASSs[k]);
    Serial.print("[WIFI] WIFI Config added: ");
    Serial.print(SSIDs[k]);
    Serial.print(" ");
    Serial.println(PASSs[k]);
  }

  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("[WIFI] Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Initialize the library with the Firebase authen and config.
  Firebase.begin(DATABASE_URL, DATABASE_SECRET);
  Firebase.reconnectWiFi(true);
  Firebase.setMaxRetry(fbdo, 3);
  Firebase.setMaxErrorQueue(fbdo, 30);
  Firebase.enableClassicRequest(fbdo, true);
  fbdo.setBSSLBufferSize(1024, 1024); //minimum size is 512 bytes, maximum size is 16384 bytes
  fbdo.setResponseSize(1024); //minimum size is 1024 bytes


  pinMode(IR_PIN, INPUT);

  timeClient.begin();
}


// -------------------------------- ARDUINO LOOP --------------------------------
void loop()
{
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  if (digitalRead(IR_PIN) == 0) {
    if (prevColorWasWhite) {
      if (!iHaveSeenBlackOnceBefore) {
        lastEnteredBlackTime = millis();
        iHaveSeenBlackOnceBefore = true;
      }
      else {
        deltaT = millis() - lastEnteredBlackTime;
        if (deltaT > 500) {
          Serial.print("deltaT: ");
          Serial.println(deltaT);

          rpm = 60000 / deltaT;
          Serial.print("RPM: ");
          Serial.println(rpm);
          Serial.println();
        }
        iHaveSeenBlackOnceBefore = false;
      }
      prevColorWasWhite = false;
    }
  } else {
    prevColorWasWhite = true;
  }

  if (millis() - lastEnteredBlackTime >= 60000) {
    rpm = 0.0;
  }

  if (rpm != rpmToSend) {
    lastSent = millis();
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    if (Firebase.setFloatAsync(fbdo, "/hamsterRPMdata/" + timeClient.getFormattedTime(), rpm)) {
      Serial.println(fbdo.dataPath());
      Serial.println(fbdo.pushName());
      Serial.println(fbdo.dataPath() + "/" + fbdo.pushName());
    } else {
      Serial.println(fbdo.errorReason());
    }

    rpmToSend = rpm;
  }
}
