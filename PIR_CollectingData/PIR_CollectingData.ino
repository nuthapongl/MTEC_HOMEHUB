const int potPin = 36;
const int COUT = 39;
int potValue = 0;
#include <SimpleKalmanFilter.h>
SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);
#include <WiFiManager.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "time.h"
#define MQTT_SERVER   "192.168.1.105"
#define MQTT_PORT     1883
#define MQTT_USERNAME "esp32_01_08583"
#define MQTT_PASSWORD "123456"
#define MQTT_NAME     "pills"
#define MQTT_MAX_MESSAGE_SIZE 1024
const char *ssid     = "NUTHAPONG_2.4G";
const char *password = "0830075461M";
//const char *ssid     = "ESICLAB";
//const char *password = "Esic@2021";
WiFiClient client;
PubSubClient mqtt(client);
StaticJsonDocument<2000> jsonBuffer;
DynamicJsonDocument doc(2000);

const int numSamples = 801;
int samplesRead = numSamples;

void setup() {
  pinMode(potPin, INPUT);
  pinMode(COUT, INPUT);
  Serial.begin(9600);


  delay(1000);
}
int minn = 10000;
void loop() {

  potValue = analogRead(potPin); // อ่านค่า Analog จากขา 34
  int C = digitalRead(COUT);

  //  while (samplesRead == numSamples) {
  //      int temp = analogRead(potPin);
  //      if (temp < 1500 || temp > 2200) {
  //        // reset the sample read count
  //        Serial.print("Start : ");
  //        Serial.println(temp);
  //        samplesRead = 0;
  //        break;
  //      }
  //
  //  }

  if (samplesRead == numSamples) {
    if (potValue < 1250 || potValue > 2250) {
      Serial.print("Start : ");
      Serial.println(potValue);
      samplesRead = 0;
    }
  }
  if (samplesRead < numSamples) {

    samplesRead++;
    Serial.println(potValue);
    //    if(potValue < minn && potValue > 1000)
    //      minn =potValue;
    //    Serial.print(" ");
    //    Serial.println(minn);
    if (samplesRead == numSamples) {
      // add an empty line if it's the last sample
      Serial.print("END :");
      Serial.println(numSamples);
      delay(10000);
    }
  }
  delay(5);

}
