#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"
#include <ArduinoJson.h>
#include<math.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define LED 2
#define REPORTING_PERIOD_MS     1000
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
const TickType_t xDelay20ms = pdMS_TO_TICKS(20);
const TickType_t xDelay100ms = pdMS_TO_TICKS(100);
const TickType_t xDelay500ms = pdMS_TO_TICKS(500);

PulseOximeter pox;
SimpleKalmanFilter HR_Kalman(2, 2, 0.01);
SimpleKalmanFilter PSO2_Kalman(2, 2, 0.01);
void TaskBeat(void *pvParameters);
void TaskMQTT(void *pvParameters);


const char *ssid = "Homehub";
const char *password = "homehub1234";
//const char *ssid = "NUTHAPONG_2.4G";
//const char *password = "0830075461M";
#define MQTT_SERVER "192.168.4.1"
#define MQTT_PORT 1883
#define MQTT_USERNAME "esp32_ble0001"
#define MQTT_PASSWORD "123456"
#define MQTT_NAME "esp32_ble0001"
#define MQTT_MAX_MESSAGE_SIZE 512

WiFiClient client;
PubSubClient mqtt(client);
DynamicJsonDocument doc(5000);
//ff:a5:21:a7:a7:d7};
unsigned long entry;
unsigned int count[3];
unsigned int foundDevice[10];
bool movingState = false;
bool detectState = false;
long startTimer = 0;
uint32_t tsLastReport = 0;
int numFound[3];
int numUnderRatio[3];
int numOverRatio = 0;
float PSO2_estimated_value;
float HR_estimated_value;
const long SERIAL_REFRESH_TIME = 100;
long refresh_time;
int countIN = 0, countUN = 0, countOUT = 0, countES = 0;

int sum = 0;
bool known = false;
int Known[3] = {0, 0, 0};
bool trig = false;
String Status[3];
long waitTime = millis();
float ratio[3], value2;
long startTime = 0;
void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  //pClient  = BLEDevice::createClient();

  Serial.println("Done");

  WiFi.begin(ssid, password);
  while ((WiFi.status() != WL_CONNECTED))
  {
    vTaskDelay(500);
    Serial.print(".");
    if (millis() - waitTime > 20000)
    {
      waitTime = millis();
      WiFi.reconnect();
    }
  }
  startTime = millis();

  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
  delay(1000);
  xTaskCreate(
    TaskBeat, "TaskBeat" // A name just for humans
    ,
    10000 // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,
    NULL);
  delay(1000);
  xTaskCreate(
    TaskMQTT, "TaskMQTT" // A name just for humans
    ,
    10000 // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,
    NULL);

  //   configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop()
{
  delay(1);
}

void TaskMQTT(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  Serial.println("Wifi Connected");
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setBufferSize(MQTT_MAX_MESSAGE_SIZE);

  for (;;)
  {

    if (mqtt.connected() == false)
    {
      Serial.print("MQTT connection... ");
      if (mqtt.connect(MQTT_NAME))
      {
        Serial.println("connected");
       // mqtt.subscribe("Lk3ya3cv/ble/station/1");
       // mqtt.publish("Lk3ya3cv/ble/station/1", "start");
      }
      else
      {
        Serial.println("failed");
      }
      while (WiFi.isConnected() != true)
      {
        Serial.println(WiFi.status());

        //WiFi.reconnect();
      }
    }
    else
    {
      mqtt.loop();
      if (trig)
      {
        trig = false;
        // char str[100]; //ratio, Status.c_str(),
        // sprintf(str, "%d,%.2f,%.2f,%s,%d,%.2f,%.2f,%s,%d,%.2f,%.2f,%s", numFound[0], ratio[0], estimated_value[0], Status[0].c_str(), numFound[1], ratio[1], estimated_value[1], Status[1].c_str(), numFound[2], ratio[2], estimated_value[2], Status[2].c_str());
        // mqtt.publish("Lk3ya3cv/ble/station/1", str);

        // “RSSI” : -78.34,
        // “Ratio”:0.7,
        // “Result”:”IN”,
        //“Found”:10

//        for (int i = 0; i < 3; i++)
//        {
//          JsonObject doc_0 = doc.createNestedObject();
//          float rssi ;
//          Serial.println(estimated_value[i]);
//          //sprintf(rssi, "%.2f",estimated_value[i]);
//          doc_0["RSSI"] = roundf(estimated_value[i] * 100) / 100.00;
//          doc_0["Ratio"] =  roundf(ratio[i] * 100) / 100.00;
//          doc_0["Result"] =  Status[i].c_str();
//          doc_0["Found"] =  numFound[i];
//          doc_0["MAC"] =  knownAddresses[i];
//          //JsonArray doc_0_t = doc_0.createNestedArray("t");
//        }
//        serializeJson(doc, Serial);
//        char buffer1[1000];
//        size_t n1 = serializeJson(doc, buffer1);
//        Serial.println("MQTT connected send temp click to server");
//        //mqtt.publish("Lk3ya3cv/ble/station/1", buffer1, n1);
//        doc.clear();
//        for (int i = 0; i < 3; i++)
//        {
//          Status[i] = "";
//          numFound[i] = 0;
//          numUnderRatio[i] = 0;
//          ratio[i] = 0.0;
//        }
      }
    }
    vTaskDelay(10);
  }
}

void TaskBeat(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  vTaskDelay(500);
  for (;;)
  {
    pox.update();

    if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        if(pox.getHeartRate() >50 && pox.getSpO2() >80 ){
           HR_estimated_value = HR_Kalman.updateEstimate(pox.getHeartRate());
           PSO2_estimated_value = PSO2_Kalman.updateEstimate(pox.getSpO2());
           Serial.println("update filter");
           // send
           
         }
        
        tsLastReport = millis();
    }

    vTaskDelay(xDelay100ms);
  }
}
