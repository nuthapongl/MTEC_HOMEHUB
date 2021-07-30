#include <Arduino.h>
#include <SimpleKalmanFilter.h>
#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"
#include <ArduinoJson.h>
#include<math.h>
#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define LED 2
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
const TickType_t xDelay20ms = pdMS_TO_TICKS(20);
const TickType_t xDelay100ms = pdMS_TO_TICKS(100);
const TickType_t xDelay500ms = pdMS_TO_TICKS(500);

void TaskScan(void *pvParameters);
void TaskMQTT(void *pvParameters);
const char *ntpServer = "th.pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 0;
SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);
static BLEAddress *pServerAddress;
BLEScan *pBLEScan;
BLEClient *pClient;
bool deviceFound = false;
const char *ssid = "Homehub";
const char *password = "homehub1234";
//const char *ssid = "NUTHAPONG_2.4G";
//const char *password = "0830075461M";
#define MQTT_SERVER "192.168.4.1"
#define MQTT_PORT 1883
#define MQTT_USERNAME "esp32_ble0002"
#define MQTT_PASSWORD "123456"
#define MQTT_NAME "esp32_ble0002"
#define MQTT_MAX_MESSAGE_SIZE 512
//4c:24:98:8a:88:b0
//b8:75:4e:de:93:c0",
//ff:a5:21:a7:a7:d7
// "7d:f4:4c:f1:a9:ec",
//4c:24:98:77:7b:de //2
//ff:a5:21:a7:a7:d7 Green-01
WiFiClient client;
PubSubClient mqtt(client);
String knownAddresses[] = {"ff:a5:21:a7:a7:d7", "b8:75:4e:de:93:c0", "4c:24:98:77:81:ce"};
DynamicJsonDocument doc(5000);
//ff:a5:21:a7:a7:d7};
unsigned long entry;
unsigned int count[3];
unsigned int foundDevice[10];
bool movingState = false;
bool detectState = false;
long startTimer = 0;

int numFound[3];
int numUnderRatio[3];
int numOverRatio = 0;
float estimated_value[3];
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
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice Device)
  {
    pServerAddress = new BLEAddress(Device.getAddress());
    // ให้ known = false
    known = false;
    int addr = -1;
    // ถ้าจับสัญญาณของอุปกรณ์ที่เรากำหนดไว้ได้ ให้ known = true
    for (int i = 0; i < (sizeof(knownAddresses) / sizeof(knownAddresses[0])); i++)
    {
      if (strcmp(pServerAddress->toString().c_str(), knownAddresses[i].c_str()) == 0)
      {
        //known = true;
        Known[i] = 1;
        addr = i;
        // Serial.println(knownAddresses[i]);
      }
      // เมื่อ known = true ทำการเช็คค่า RSSI ถ้าถึงค่าที่กำหนด DeviceFound = true ถ้ายังไม่ถึง(เข้าใกล้ไม่พอ) DeviceFound = false
    }

    // เมื่อ known = true ทำการเช็คค่า RSSI ถ้าถึงค่าที่กำหนด DeviceFound = true ถ้ายังไม่ถึง(เข้าใกล้ไม่พอ) DeviceFound = false
    for (int j = 0; j < 3; j++)
    {
      if (Known[j] == 1)
      {
        Known[j] = 0;
        // known = false;
        count[j]++;
        //Serial.print(pServerAddress->toString().c_str());
        //        Serial.print(" : Device found: ");
        estimated_value[j] = simpleKalmanFilter.updateEstimate(Device.getRSSI());
        Serial.print(Device.getRSSI());
        Serial.print(" , ");
        numFound[j]++;
        // สามารถเปลี่ยนค่า RSSI ตรงนี้ได้ตามต้องการ
        if (Device.getRSSI() >= -78)
        {
          deviceFound = true;
          foundDevice[j]++;
          numUnderRatio[j]++;
        }
        else
        {
          deviceFound = false;
          numOverRatio++;
        }
        //Device.getScan()->stop();
        pBLEScan->erase(Device.getAddress());
        //delay(20);
      }
    }
    free(pServerAddress);
  }
};

int scanTime = 10;
// ฟังก์ชั่นการทำงาน ถ้า DeviceFound = true ให้ LED เปิด และถ้า DeviceFound = false ให้ LED ปิด
void Working()
{
  // Serial.println();
  startTimer = millis();
  //Serial.println("BLE Scan restarted.....");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  MyAdvertisedDeviceCallbacks myCallbacks;
  pBLEScan->setAdvertisedDeviceCallbacks(&myCallbacks);
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(1000);
  pBLEScan->setWindow(500);
  deviceFound = false;
  pBLEScan->start(scanTime);
  //pBLEScan
  //Serial.println(ESP.getFreeHeap());
  //
  //free(scanResults);
  // bool test = pBLEScan->start(scanTime,scanCompleteCB,true );
  Serial.println("end");
  Serial.println(ESP.getFreeHeap());
  Serial.print("found : ");
  for (int i = 0; i < 3; i++)
  {
    Serial.print(knownAddresses[i]);
    Serial.print("   :  ");
    Serial.print(count[i]);
    Serial.print(",");
    Serial.println(foundDevice[i]);
    foundDevice[i] = count[i] = 0;
  }
  //free(pBLEScan);
  //Serial.println(ESP.getFreeHeap());
  //BLEDevice::deinit(true);
  //pBLEScan->clearResults();
  //pBLEScan->stop();

  //delay(50);
}
long startTime = 10;

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  //pClient  = BLEDevice::createClient();

    Serial.println("Beacon station -2 , Version 1.1 (edit retain msg )");

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

  delay(1000);
  xTaskCreate(
      TaskScan, "TaskScan" // A name just for humans
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
        mqtt.subscribe("Lk3ya3cv/ble/station/2");
        mqtt.publish("Lk3ya3cv/ble/station/2", "start");
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

        for (int i = 0; i < 3; i++)
        {
          JsonObject doc_0 = doc.createNestedObject();
          float rssi ;
          Serial.println(estimated_value[i]);
          //sprintf(rssi, "%.2f",estimated_value[i]);
          doc_0["RSSI"] = roundf(estimated_value[i]*100)/100.00;
           doc_0["Ratio"] =  roundf(ratio[i]*100)/100.00;
           doc_0["Result"] =  Status[i].c_str();
           doc_0["Found"] =  numFound[i];
           doc_0["MAC"] =  knownAddresses[i];
          //JsonArray doc_0_t = doc_0.createNestedArray("t");
        }
        serializeJson(doc, Serial);
        char buffer1[1000];
        size_t n1 = serializeJson(doc, buffer1);
        Serial.println("MQTT connected send temp click to server");
        mqtt.beginPublish("Lk3ya3cv/ble/station/1", measureJson(doc), 0);
        serializeJson(doc, mqtt);
        mqtt.endPublish();
        doc.clear();
        for (int i = 0; i < 3; i++)
        {
          Status[i] = "";
          numFound[i] = 0;
          numUnderRatio[i] = 0;
          ratio[i] = 0.0;
        }
      }
    }
    vTaskDelay(10);
  }
}

void TaskScan(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  vTaskDelay(500);
  for (;;)
  {

    if (millis() - startTime > 10000)
    {

      startTime = millis();

      //      Serial.println(numFound);
      for (int k = 0; k < 3; k++)
      {

        if (numFound[k] != 0)
        {
          ratio[k] = numUnderRatio[k] * 1.0 / numFound[k];
          value2 = numOverRatio * 1.0 / numFound[k];
          sum = sum / numFound[k];
        }
        else
        {
          ratio[k] = value2 = 0;
          sum = 0;
        }
        //Serial.print("\ncount IN : "); Serial.print(countIN); Serial.print(",OUT : "); Serial.print(countOUT); Serial.print(",UN : "); Serial.print(countUN); Serial.print(",ES : "); Serial.println(countES);
        //Serial.println("Found : " + numFound + " , IN : " + numUnderRatio + " , OUT : " + numOverRatio);
        Serial.print("Found : ");
        Serial.println(numFound[k]); // Serial.print(" , IN : "); Serial.print(numUnderRatio); Serial.print(" , OUT : "); Serial.println(numOverRatio);
        Serial.print("Ratio : ");
        Serial.print(ratio[k]);
        Serial.print(" , kal : ");
        Serial.println(estimated_value[k]);
        sum = 0;
        if (numFound[k] >= 5)
        {
          if (ratio[k] >= 0.8)
          {
            Status[k] = "IN";
            Serial.println("Result : In");
            countIN++;
          }
          else if (ratio[k] > 0.2)
          {

            Serial.println("Result : Uncertain");
            if (estimated_value[k] > -80)
            {
              countES++;
              Status[k] = "IN";
              Serial.println("OUTPUT : IN");
            }
            else
            {
              Status[k] = "UN";
              Serial.println("OUTPUT : Uncertain");
              countUN++;
            }
          }
          else if (ratio[k] <= 0.2 && ratio[k] >= 0)
          {
            Status[k] = "OUT";
            Serial.println("Result : Out");
            countOUT++;
          }
        }
        else if (numFound[k] < 5)
        {
          Status[k] = "OUT";
          Serial.println("Result : Out");
          countOUT++;
          //      if (value == 1.0) {
          //        Serial.println("Result : In");
          //      }
          //      else if (value == 0.0 || numFound ==0) {
          //
          //      }
          //      else{
          //        Serial.println("Result : Uncertain");
          //       }
        }
        //    if (numFound > 5) {
        //      if (value >= 0.6) {
        //        Serial.println("Result : Ibeacon in range");
        //      }
        //      else {
        //        Serial.println("Result : Ibeacon out of range");
        //      }
        //    }
        //    else if (numFound > 0 && numFound <= 5) {
        //      if (value >= 0.8) {
        //        Serial.println("Result : Ibeacon in range");
        //      }
        //      else if(value2 >=0.8){
        //        Serial.println("Result : Ibeacon out of range");
        //      }
        //      else {
        //        Serial.println("Result : undecide");
        //      }
        //    }
        //
        //    else if( numFound == 0){
        //      Serial.println("Result : Ibeacon out of range");
        //    }
        //printLocalTime();

        if ((countIN + countOUT + countUN + countES) >= 99)
        {
          countIN = countOUT = countUN = countES = 0;
        }
      }
      trig = true;
    }
    //Serial.println("ddd");
    Working();
    vTaskDelay(xDelay100ms);
  }
}
