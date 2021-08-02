#include <VL53L1X.h> //Download this library from https://github.com/pololu/vl53l1x-arduino
#include <Wire.h>
#include <FastLED.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#define Sensor1_newAddress 33 //17
#define Sensor2_newAddress 39
#define Button_PIN 16
#define NUM_LEDS 8
#define DATA_PIN 17
//#define WIFI_STA_NAME "Homehub"
//#define WIFI_STA_PASS "homehub1234"
#define WIFI_STA_NAME "NUTHAPONG_2.4G"
#define WIFI_STA_PASS "0830075461M"

#define MQTT_SERVER   "68.183.224.46"
#define MQTT_PORT     8883
#define MQTT_USERNAME "esp32_Gate_123e"
#define MQTT_PASSWORD "123456"
#define MQTT_NAME     "Gate_001"
#define XSHUT_A 18
#define XSHUT_B 19
#define M_INTERVAL 45
const int limit_range = 650;
const unsigned int interval_mqtt = 300000;

CRGB leds[NUM_LEDS];
int trigL[4];
int trigR[4];
long Start_timeL = 0;
long Start_timeR = 0;

VL53L1X sensor_A; //Create the sensor object
VL53L1X sensor_B; //Create the sensor object
WiFiClient client;
PubSubClient mqtt(client);
DynamicJsonDocument doc(5000);

int mInterval = 55; //refresh rate of 10hz
int Start = 0;
char buf[40];
long Restart_Time = 0;
int state_button = 0;
int Status = 0;
double duration = 0.0;
unsigned long startTime = 0;
unsigned long previousTime_mqtt = 0;
int error_count = 0;
int distance[2] = {0, 0};

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String topic_str = topic, payload_str = (char*)payload;
  // Serial.println("[" + topic_str + "]: " + payload_str);
  //Serial.println(WiFi.status());
  //digitalWrite(LED_PIN, (payload_str == "ON") ? HIGH : LOW);
  /* if (topic_str == "/gate/01") {
     //Serial.println("Correct");
     if (payload_str == "in") {
       Serial.println("IN");

     }
    }*/

}

void setup(void)
{
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);

  pinMode(Button_PIN, INPUT);
  pinMode(XSHUT_A , OUTPUT);
  pinMode(XSHUT_B , OUTPUT);
  digitalWrite(XSHUT_A, LOW);
  digitalWrite(XSHUT_B, LOW);

  delay(50);
  digitalWrite(XSHUT_A, HIGH); //Turn sensor_A on
  delay(50);

  sensor_A.setTimeout(500); //Set the sensors timeout

  if (!sensor_A.init())//try to initilise the sensor
  {
    //Sensor does not respond within the timeout time
    Serial.println("Sensor_A is not responding, check your wiring");
  }
  else
  {
    sensor_A.setAddress(42); //Set the sensors I2C address
    sensor_A.setDistanceMode(VL53L1X::Short); //Set the sensor to maximum range of 4 meters
    sensor_A.setMeasurementTimingBudget(40000); //Set its timing budget in microseconds longer timing budgets will give more accurate measurements
    sensor_A.startContinuous(M_INTERVAL); //Sets the interval where a measurement can be requested in milliseconds
  }

  delay(50);
  digitalWrite(XSHUT_B, HIGH); //Turn sensor_A on
  delay(50);

  sensor_B.setTimeout(500); //Set the sensors timeout

  if (!sensor_B.init())//try to initilise the sensor
  {
    //Sensor does not respond within the timeout time
    Serial.println("Sensor_A is not responding, check your wiring");
  }
  else
  {
    // sensor_B.setAddress(43); //Set the sensors I2C address
    sensor_B.setDistanceMode(VL53L1X::Short); //Set the sensor to maximum range of 4 meters
    sensor_B.setMeasurementTimingBudget(40000); //Set its timing budget in microseconds longer timing budgets will give more accurate measurements
    sensor_B.startContinuous(M_INTERVAL); //Sets the interval where a measurement can be requested in milliseconds
  }
  delay(50);
  Scanner ();
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  leds[0] = CRGB::Pink;
  FastLED.show();
  delay(500);
  Start = millis();
  Serial.print("Connecting to ");
  Serial.println(WIFI_STA_NAME);


  WiFi.begin(WIFI_STA_NAME, WIFI_STA_PASS);
  while ((WiFi.status() != WL_CONNECTED))
  {
    delay(500);
    Serial.print(".");

  }
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(callback);

  leds[0] = CRGB::Blue;
  FastLED.show();
  Restart_Time = millis();
}



void loop(void)
{

  //  if (millis() - Restart_Time  > 1800000 ) {
  //    ESP.restart();
  //  }
  int a = !digitalRead(Button_PIN);
  //Serial.println(a);
  if (state_button == 0 && a == 0) {
    state_button = 1;

  }
  else if (state_button == 1 && a == 1) {
    state_button = 0;
    delay(200);
  }

  if (mqtt.connected() == false) {
    Serial.print("MQTT connection... ");
    if (mqtt.connect(MQTT_NAME)) {
      Serial.println("connected");
    } else {
      Serial.println("failed");
      delay(5000);
    }
    while (WiFi.isConnected() != true) {
      Serial.println(WiFi.status());
      WiFi.reconnect();
      delay(2000);
    }

  } else {
    mqtt.loop();
  }

  int startT = millis();
  sensor_A.startContinuous(mInterval);
  while (!sensor_A.dataReady()) {
    int a = sensor_A.dataReady();
    if (a != 0)
      Serial.println(a);
    delay(1);
  }
  distance[0] = sensor_A.read();
  Serial.print("Sensor_A Reading: ");
  Serial.print(distance[0]);
  sensor_A.stopContinuous();

  sensor_B.startContinuous(mInterval);
  while (!sensor_B.dataReady()) {
    int b = sensor_B.dataReady();
    if (b != 0)
      Serial.println(b);
    delay(1);
  }
  distance[1] = sensor_B.read();
  Serial.print(" Sensor_B Reading: ");
  Serial.println(distance[1]);
  sensor_A.stopContinuous();

  Serial.printf("total time : %d \n", millis() - startT);
  //  if ((millis() - startTime) > mInterval)
  //  {
  //    distance1 = sensor_A.read(); //Get the result of the measurement from the sensor
  //    distance2 = sensor_B.read();
  //    //    Serial.print("Sensor_A Reading: ");
  //    //    Serial.print(distance1); //Get a reading in millimeters
  //    //   // Serial.print("Sensor_B Reading: ");
  //    //    Serial.print("\t");
  //    //    Serial.println(distance2);
  //    startTime = millis();
  //  }


  if (distance[0] > limit_range) {
    distance[0] = limit_range;
  }
  if (distance[1] > limit_range) {
    distance[1] = limit_range;
  }
  if (!(distance[0] == 650 && distance[1] == 650)) {
    //Serial.printf("dis : %d , %d\n",  distance1 ,  distance2 );
  }
  if (trigL[0] == 1) {
    Serial.print("trig L :");
    for (int k = 0; k < 4; k++) {
      Serial.printf("%d ", trigL[k]);
    }
    Serial.println();
  }
  if (trigR[0] == 1) {
    Serial.print("trig R :");
    for (int k = 0; k < 4; k++) {
      Serial.printf("%d ", trigR[k]);
    }
    Serial.println();
  }
  if (distance[0] >= limit_range && distance[1] >= limit_range) {
    leds[0] = CRGB::Black;
    leds[7] = CRGB::Black;
    FastLED.show();
  }

  if (distance[0] < limit_range - 50 && trigL[0] == 0 && trigR[0] == 0  && distance[0] < distance[1] ) {
    trigL[0] = 1;
    Start_timeL = millis();
    Serial.print("Start Left : ");
    Serial.println(Start_timeL);
  }
  else  if (distance[1] < limit_range - 50 && trigR[0] == 0 && trigL[0] == 0 && distance[1] < distance[0]) {
    trigR[0] = 1;
    Start_timeR = millis();
    Serial.print("Start Right : ");
    Serial.println(Start_timeR);
  }
  if (trigL[0] == 1 && trigR[0] == 0) {
    if (distance[1] < distance[0] + 50  ) {
      trigL[1] = 1;
      //     Serial.println("trigL1 ");
    }
    if (((distance[0] >= distance[1] + 50) || (distance[0] <= distance[1] + 50) ) && trigL[1] == 1 ) {
      trigL[2] = 1;
      // Serial.println("trigL2 ");
    }
    if ( distance[0] > distance[1]   && trigL[1] == 1 && trigL[2] == 1) {
      trigL[3] = 1;
      // Serial.println("trigL3 ");
    }
  }

  if (trigR[0] == 1 && trigL[0] == 0) {
    if (distance[0] < distance[1] + 50  ) {
      trigR[1] = 1;
      // Serial.println("trigR1 ");
    }
    if (((distance[1] >= distance[0] + 50) || (distance[1] <= distance[0] + 50) ) && trigR[1] == 1 ) {
      trigR[2] = 1;
      // Serial.println("trigR2 ");
    }
    if ( distance[1] > distance[0]    && trigR[1] == 1 && trigR[2] == 1) {
      trigR[3] = 1;
      // Serial.println("trigR3 ");
    }
  }

  // Serial.printf("%d %d %d %d ==", trigL[0], trigL[1], trigL[2], trigL[3]);
  // Serial.printf("%d %d %d %d\n", trigR[0], trigR[1], trigR[2], trigR[3]);
  if (trigL[0] && trigL[1] && trigL[2] && trigL[3]) {

    leds[0] = CRGB::Green;
    FastLED.show();
    Start_timeL = 0;
    //delay(1000);
    if (distance[0] >=  limit_range && distance[1] >= limit_range) {
      Serial.println("1 > 2");
      if (Status == 0) {
        previousTime_mqtt = startTime = millis();
        Status = 1;

        Serial.printf("Status: On going, duration: 0  minutes.\n");

        doc["deviceId"] = "9da011eb-00ee-47ae-b055-22c58a2985cc";
        doc["status"] = "ongoing";
        doc["duration"] = 0;
        serializeJson(doc, Serial);

        char buffer2[200];
        size_t n2 = serializeJson(doc, buffer2);
        mqtt.beginPublish("gate/status", measureJson(doc), 0);
        serializeJson(doc, mqtt);
        mqtt.endPublish();
        doc.clear();
      }
      //  Serial.println("reset");
      doc["deviceId"] = "9da011eb-00ee-47ae-b055-22c58a2985cc";
      doc["direction"] = "in";
      Serial.printf("error : %d\n", error_count);
      serializeJson(doc, Serial);
      char buffer1[200];
      size_t n1 = serializeJson(doc, buffer1);
      mqtt.beginPublish("gate", measureJson(doc), 0);
      serializeJson(doc, mqtt);
      mqtt.endPublish();
      doc.clear();
      for (int i = 0; i < 4; i++) {
        trigL[i] = 0;
        trigR[i] = 0;
      }
    }
  }
  else if (trigR[0] && trigR[1] && trigR[2] && trigR[3]) {

    leds[0] = CRGB::DeepPink;
    FastLED.show();
    //delay(1000);
    Start_timeR = 0;

    if (Status == 1) {
      Status = 2;
      duration = convertTime((millis() - startTime) / 1000);
      Serial.printf("Status: Complete, duration: %.2f minutes.\n", duration);



      doc["deviceId"] = "9da011eb-00ee-47ae-b055-22c58a2985cc";
      doc["status"] = "complete";
      doc["duration"] = duration;
      serializeJson(doc, Serial);
      char buffer2[200];
      size_t n2 = serializeJson(doc, buffer2);
      mqtt.beginPublish("gate/status", measureJson(doc), 0);
      serializeJson(doc, mqtt);
      mqtt.endPublish();
      doc.clear();
      Status = 0;
      duration = 0;
      previousTime_mqtt = 0;
    }
    if (distance[0] >=  limit_range && distance[1] >= limit_range) {
      Serial.println("2 > 1");
      doc["deviceId"] = "9da011eb-00ee-47ae-b055-22c58a2985cc";
      doc["direction"] = "out";
      Serial.printf("error : %d\n", error_count);
      serializeJson(doc, Serial);

      char buffer2[200];
      size_t n2 = serializeJson(doc, buffer2);
      mqtt.beginPublish("gate/direction", measureJson(doc), 0);
      serializeJson(doc, mqtt);
      mqtt.endPublish();
      doc.clear();
      for (int i = 0; i < 4; i++) {
        trigR[i] = 0;
        trigL[i] = 0;
      }
    }
  }


  if (trigL[0] == 1 && distance[0] >= limit_range && distance[1] >= limit_range && (millis() - Start_timeL  > 1500)) {
    Serial.println("clear L ");
    for (int i = 0; i < 4; i++)
      trigL[i] = 0;
    error_count ++;
  }
  if (trigR[0] == 1 && distance[0] >= limit_range && distance[1] >= limit_range && (millis() - Start_timeR  > 1500)) {
    Serial.println("clear R ");
    for (int i = 0; i < 4; i++)
      trigR[i] = 0;
    error_count ++;
  }

  if (Status == 1 && millis() - previousTime_mqtt >= interval_mqtt) {
    previousTime_mqtt = millis();
    // duration += 5.0;
    duration = convertTime((millis() - startTime) / 1000);
    Serial.printf("Status: On going, duration: %.2f minutes.\n", duration);

    doc["deviceId"] = "9da011eb-00ee-47ae-b055-22c58a2985cc";
    doc["status"] = "ongoing";
    doc["duration"] = duration;
    serializeJson(doc, Serial);

    char buffer2[200];
    size_t n2 = serializeJson(doc, buffer2);
    mqtt.beginPublish("gate/status", measureJson(doc), 0);
    serializeJson(doc, mqtt);
    mqtt.endPublish();
    doc.clear();

  }
  delay(1);
}

float convertTime(uint32_t secs) {
  uint32_t m, s;
  m = secs / 60;
  s = secs % 60;
  float result = m + (s / 100.0);
  Serial.printf("result . %.2f ", result);
  return result;
}

void Scanner ()
{
  Serial.println ();
  Serial.println ("I2C scanner. Scanning ...");
  byte count = 0;

  Wire.begin();
  for (byte i = 8; i < 120; i++)
  {
    Wire.beginTransmission (i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission () == 0)  // Receive 0 = success (ACK response)
    {
      Serial.print ("Found address: ");
      Serial.print (i, DEC);
      Serial.print (" (0x");
      Serial.print (i, HEX);     // PCF8574 7 bit address
      Serial.println (")");
      count++;
    }
  }
  Serial.print ("Found ");
  Serial.print (count, DEC);        // numbers of devices
  Serial.println (" device(s).");
}
