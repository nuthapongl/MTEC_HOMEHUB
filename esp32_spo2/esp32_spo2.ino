
#include <Arduino.h>
#include <Adafruit_GFX.h>        //OLED libraries
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "time.h"
#include <ArduinoJson.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif
#define REPORTING_PERIOD_MS     1000
const TickType_t xDelay2000ms = pdMS_TO_TICKS(2000);
const TickType_t xDelay20ms = pdMS_TO_TICKS(20);
const TickType_t xDelay100ms = pdMS_TO_TICKS(100);
const TickType_t xDelay500ms = pdMS_TO_TICKS(500);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //Declaring the display name (display)

static const unsigned char PROGMEM logo2_bmp[] =
{ 0x03, 0xC0, 0xF0, 0x06, 0x71, 0x8C, 0x0C, 0x1B, 0x06, 0x18, 0x0E, 0x02, 0x10, 0x0C, 0x03, 0x10,              //Logo2 and Logo3 are two bmp pictures that display on the OLED if called
  0x04, 0x01, 0x10, 0x04, 0x01, 0x10, 0x40, 0x01, 0x10, 0x40, 0x01, 0x10, 0xC0, 0x03, 0x08, 0x88,
  0x02, 0x08, 0xB8, 0x04, 0xFF, 0x37, 0x08, 0x01, 0x30, 0x18, 0x01, 0x90, 0x30, 0x00, 0xC0, 0x60,
  0x00, 0x60, 0xC0, 0x00, 0x31, 0x80, 0x00, 0x1B, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x04, 0x00,
};

static const unsigned char PROGMEM logo3_bmp[] =
{ 0x01, 0xF0, 0x0F, 0x80, 0x06, 0x1C, 0x38, 0x60, 0x18, 0x06, 0x60, 0x18, 0x10, 0x01, 0x80, 0x08,
  0x20, 0x01, 0x80, 0x04, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0xC0, 0x00, 0x08, 0x03,
  0x80, 0x00, 0x08, 0x01, 0x80, 0x00, 0x18, 0x01, 0x80, 0x00, 0x1C, 0x01, 0x80, 0x00, 0x14, 0x00,
  0x80, 0x00, 0x14, 0x00, 0x80, 0x00, 0x14, 0x00, 0x40, 0x10, 0x12, 0x00, 0x40, 0x10, 0x12, 0x00,
  0x7E, 0x1F, 0x23, 0xFE, 0x03, 0x31, 0xA0, 0x04, 0x01, 0xA0, 0xA0, 0x0C, 0x00, 0xA0, 0xA0, 0x08,
  0x00, 0x60, 0xE0, 0x10, 0x00, 0x20, 0x60, 0x20, 0x06, 0x00, 0x40, 0x60, 0x03, 0x00, 0x40, 0xC0,
  0x01, 0x80, 0x01, 0x80, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x30, 0x0C, 0x00,
  0x00, 0x08, 0x10, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00, 0x03, 0xC0, 0x00, 0x00, 0x01, 0x80, 0x00
};
#define REPORTING_PERIOD_MS     1000
#define MQTT_SERVER "192.168.4.1"
#define MQTT_PORT 1883
#define MQTT_USERNAME "esp32_psjho2"
#define MQTT_PASSWORD "123456"
#define MQTT_NAME "esp32_pso7_8987"
#define MQTT_MAX_MESSAGE_SIZE 512

PulseOximeter pox;
void TaskMQTT(void *pvParameters);

#define WIFI_STA_NAME "Homehub"
#define WIFI_STA_PASS "homehub1234"
//#define WIFI_STA_NAME "Tuamang-Iphone"
//#define WIFI_STA_PASS "12345677"
//const char *ssid = "NUTHAPONG_2.4G";
//const char *password = "0830075461M";
uint32_t tsLastReport = 0;
int hr, spo2;
int HR[20], SPO2[20];
int avg_hr, avg_spo2;
int json_hr, json_spo2;
int n = 0;
int numCount = 0;
bool trig = false;
long waitTime = millis();
WiFiClient client;
PubSubClient mqtt(client);
DynamicJsonDocument doc(5000);
void onBeatDetected()
{
  Serial.println("Beat!");
}

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //Start the OLED display
  display.display();
  Serial.begin(115200);
  delay(3000);
  // Initialize sensor


  for (int i = 0; i < 20; i++) {
    HR[i] = 0 ;
    SPO2[i] = 0;
  }
//  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);
//  while ((WiFi.status() != WL_CONNECTED))
//  {
//    delay(500);
//    Serial.print(".");
//    //    if (millis() - waitTime > 20000)
//    //    {
//    //      waitTime = millis();
//    //     // WiFi.reconnect();
//    //    }
//  }
  delay(1000);
  xTaskCreate(
    TaskMQTT, "TaskMQTT" // A name just for humans
    ,
    5000 // This stack size can be checked & adjusted by reading the Stack Highwater
    ,
    NULL, 1 // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,
    NULL);
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
  }
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  pox.setOnBeatDetectedCallback(onBeatDetected);
  Serial.println("start");
}

void loop() {
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    hr = pox.getHeartRate();
    spo2 = pox.getSpO2();
    Serial.print("Heart rate:");
    Serial.print(hr);
    Serial.print("bpm / SpO2:");
    Serial.print(spo2);
    Serial.println("%");

    tsLastReport = millis();
    if (hr == 0 && spo2 == 0 && numCount > 0) {
      Serial.println("send");
      for (int i = 0; i < numCount; i++) {
        avg_hr += HR[i];
        avg_spo2 += SPO2[i];
        Serial.printf("%d , %d\n", HR[i], SPO2[i]);
      }
      avg_hr = avg_hr / numCount ;
      avg_spo2 = avg_spo2 / numCount;
      Serial.print("AVG     Heart rate:");
      Serial.print(avg_hr);
      Serial.print("bpm / SpO2:");
      Serial.print(avg_spo2);
      Serial.println("%");
      json_hr = avg_hr;
      json_spo2 = avg_spo2;
      trig = true;

    }
    if (hr > 20 && spo2 > 80 && spo2 <= 100) {
      numCount++;
      if (numCount < 20) {
        HR[numCount - 1] = hr;
        SPO2[numCount - 1] = spo2;
      }
      else {
        for (int i = 0; i < 20; i++) {
          HR[i] = 0 ;
          SPO2[i] = 0;
        }
        numCount = -1;
      }
    }
    else {
      for (int i = 0; i < 20; i++) {
        HR[i] = 0 ;
        SPO2[i] = 0;
      }
      numCount = -1;
    }
  }
  if (hr > 20 && spo2 > 80 && spo2 <= 100 ) {
    n++;
    display.clearDisplay();
    if (n % 15 > 8)
      display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);
    else
      display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);       //Draw the first bmp picture (little heart)
    display.setTextSize(1);                                   //Near it display the average BPM you can display the BPM if you want
    display.setTextColor(WHITE);
    display.setCursor(50, 0);
    display.println("BPM");
    display.setCursor(90, 0);
    display.println("SpO2");
    display.setTextSize(2);
    display.setCursor(50, 18);
    display.println(hr);
    display.setCursor(90, 18);
    display.print(spo2);
    display.println("%");
    display.display();
    // vTaskDelay(xDelay20ms);
  }
  if (hr < 20 || spo2 < 30) {      //If no finger is detected it inform the user and put the average BPM to 0 or it will be stored for the next measure

    numCount = 0;
    avg_spo2 = 0;
    avg_hr = 0;
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30, 10);
    display.println("Please Place ");
    display.setCursor(15, 22);
    display.println("your finger 10 sec ");
    display.display();

  }


}

void TaskMQTT(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  Serial.println("Wifi Connected");
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  //mqtt.setBufferSize(MQTT_MAX_MESSAGE_SIZE);

  for (;;)
  {
    if (mqtt.connected() == false)
    {
      Serial.print("MQTT connection... ");
      if (mqtt.connect(MQTT_NAME))
      {
        Serial.println("connected");
        //mqtt.subscribe("spo2");
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
        doc["deviceId"] = "be00b084-5f92-41d2-814d-4cdd39bcd44a";
        doc["SPO2"] = json_spo2;
        doc["HR"] =  json_hr;
        serializeJson(doc, Serial);
        char buffer1[500];
        boolean retain = false;
        size_t n1 = serializeJson(doc, buffer1);
        Serial.println("MQTT send to server");
       // mqtt.publish("spo2", buffer1, n1, false);
        mqtt.beginPublish("spo2", measureJson(doc), 0);
        serializeJson(doc, mqtt);
        mqtt.endPublish();
        doc.clear();
      }
    }
  }
}
