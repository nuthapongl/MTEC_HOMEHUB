
// rev 1.0
#include <Wire.h>
#include <VL53L1X.h>

VL53L1X sensor;
//test
const unsigned int interval_mqtt = 300000;
int ROI_X[2] = {5, 5};
int ROI_Y[2] = {16, 16};
int ROI_C[2] = {150, 239};
int distance[2] = {0, 0};
int limit_range = 650;
int trigL[4];
int trigR[4];

long Start_timeL = 0;
long Start_timeR = 0;

int mInterval = 45; //refresh rate of 10hz
long Restart_Time = 0;
int detect_distance = 0;
int state_button = 0;
int Status = 0;
double duration = 0.0;
unsigned long startTime = 0;
unsigned long previousTime_mqtt = 0;
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1);
  }

  sensor.setDistanceMode(VL53L1X::Short);
  sensor.setMeasurementTimingBudget(40000);

  sensor.startContinuous(45);

  delay(500);
  Serial.println(sensor.getROICenter());
  //Serial.println(sensor.getROISize(16,16));
  delay(1000);
}

void loop()
{

  for (int i = 0; i < 2; i++) {
    sensor.setROISize(ROI_X[i], ROI_Y[i]);
    sensor.setROICenter(ROI_C[i]);
    // Serial.println(sensor.getROICenter());
    //delay(1);
    sensor.read();
    distance[i] = sensor.ranging_data.range_mm;
  }
  //Serial.printf("%d %d \n", distance[0], distance[1]);

  if (distance[0] < limit_range - 50 && trigL[0] == 0 && trigR[0] == 0 ) {
    trigL[0] = 1;
    Start_timeL = millis();
    Serial.print(trigL[0]);
    Serial.print("Start Left : ");
    Serial.println(Start_timeL);
  }
  else  if (distance[1] < limit_range - 50 && trigR[0] == 0 && trigL[0] == 0) {
    trigR[0] = 1;
    Start_timeR = millis();
    Serial.print("Start Right : ");
    Serial.println(Start_timeR);
  }
  if (trigL[0] == 1 && trigR[0] == 0) {
    //     Serial.print(String("trigL :") +trigL[0]);
    if (distance[1] < distance[0] + 150  ) {
      trigL[1] = 1;
      // Serial.println("trigL1 ");
    }
    if (((distance[0] >= distance[1] + 150) || (distance[0] <= distance[1] + 150) ) && trigL[1] == 1 ) {
      trigL[2] = 1;
      //Serial.println("trigL2 ");
    }
    if ( distance[0] > distance[1] + 150   && trigL[1] == 1 && trigL[2] == 1) {
      trigL[3] = 1;
      //Serial.println("trigL3 ");
    }
  }

  if (trigR[0] == 1 && trigL[0] == 0) {
    if (distance[0] < distance[1] + 150  ) {
      trigR[1] = 1;
      // Serial.println("trigR1 ");
    }
    if (((distance[1] >= distance[0] + 150) || (distance[1] <= distance[0] + 150) ) && trigR[1] == 1 ) {
      trigR[2] = 1;
      // Serial.println("trigR2 ");
    }
    if ( distance[1] > distance[0] + 150   && trigR[1] == 1 && trigR[2] == 1) {
      trigR[3] = 1;
      // Serial.println("trigR3 ");
    }
  }


  if (trigL[0] && trigL[1] && trigL[2] && trigL[3]) {

    Start_timeL = 0;
    //delay(1000);
    if (distance[0] >=  limit_range && distance[1] >= limit_range) {

      Serial.println("2>1 = IN");
      if (Status == 0) {
        previousTime_mqtt = startTime = millis();
        Status = 1;
      }



      //      //  Serial.println("reset");
      //      doc["deviceId"] = "9da011eb-00ee-47ae-b055-22c58a2985cc";
      //      doc["direction"] = "out";
      //      serializeJson(doc, Serial);
      //      char buffer1[200];
      //      size_t n1 = serializeJson(doc, buffer1);
      //      mqtt.beginPublish("gate", measureJson(doc), 0);
      //      serializeJson(doc, mqtt);
      //      mqtt.endPublish();
      //      doc.clear();
      for (int i = 0; i < 4; i++) {
        trigL[i] = 0;
        trigR[i] = 0;
      }
      delay(1000);
    }
  }

  else if (trigR[0] && trigR[1] && trigR[2] && trigR[3]) {

    Start_timeR = 0;
    if (distance[0] >=  limit_range && distance[1] >= limit_range) {
      Serial.println("1 >2 = OUT");
      if (Status == 1) {
        Status = 2;
        duration = convertTime((millis() - startTime) / 1000);
        Serial.printf("Status: Complete, duration: %.2f minutes.\n", duration);
        Status = 0;
        duration = 0;
        previousTime_mqtt = 0;
      }


      //      doc["deviceId"] = "9da011eb-00ee-47ae-b055-22c58a2985cc";
      //      doc["direction"] = "in";
      //      serializeJson(doc, Serial);
      //      char buffer2[200];
      //      size_t n2 = serializeJson(doc, buffer2);
      //      mqtt.beginPublish("gate", measureJson(doc), 0);
      //      serializeJson(doc, mqtt);
      //      mqtt.endPublish();
      //      doc.clear();
      for (int i = 0; i < 4; i++) {
        trigR[i] = 0;
        trigL[i] = 0;
      }
      delay(1000);
    }
  }
  // add time
  if (trigL[0] == 1 && distance[0] >= limit_range && distance[1] >= limit_range && (millis() - Start_timeL  > 1500)) {
    Serial.println("clear L ");
    for (int i = 0; i < 4; i++)
      trigL[i] = 0;
  }
  if (trigR[0] == 1 && distance[0] >= limit_range && distance[1] >= limit_range && (millis() - Start_timeR  > 1500)) {
    Serial.println("clear R ");
    for (int i = 0; i < 4; i++)
      trigR[i] = 0;
  }


  if (Status == 1 && millis() - previousTime_mqtt >= interval_mqtt) {
    previousTime_mqtt = millis();
    // duration += 5.0;
    duration = convertTime((millis() - startTime) / 1000);
    Serial.printf("Status: On going, duration: %.2f minutes.\n", duration);
  }
}

float convertTime(uint32_t secs) {
  uint32_t m, s;
  m = secs / 60;
  s = secs % 60;
  float result = m + (s / 100.0);
  Serial.printf("result . %.2f ", result);
  return result;
}
