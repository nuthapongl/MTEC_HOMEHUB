
#if !(defined(ESP8266) || defined(ESP32))
#error This code is intended to run on the ESP8266 or ESP32 platform! Please check your Tools->Board setting.
#endif

#define ESP_WIFIMANAGER_VERSION_MIN_TARGET "ESP_WiFiManager v1.7.2"

// Use from 0 to 4. Higher number, more debugging messages and memory usage.
#define _WIFIMGR_LOGLEVEL_ 3

#include <Arduino.h>   // for button
#include <OneButton.h> // for button

#include <FS.h>

// Now support ArduinoJson 6.0.0+ ( tested with v6.15.2 to v6.16.1 )
#include <ArduinoJson.h> // get it from https://arduinojson.org/ or install via Arduino library manager

//For ESP32, To use ESP32 Dev Module, QIO, Flash 4MB/80MHz, Upload 921600
//Ported to ESP32
#ifdef ESP32
#include <esp_wifi.h>
#include <WiFi.h>
#include <WiFiClient.h>

// From v1.1.0
#include <WiFiMulti.h>
WiFiMulti wifiMulti;

// LittleFS has higher priority than SPIFFS
#if (ARDUINO_ESP32C3_DEV)
// Currently, ESP32-C3 only supporting SPIFFS and EEPROM. Will fix to support LittleFS
#define USE_LITTLEFS false
#define USE_SPIFFS true
#else
#define USE_LITTLEFS true
#define USE_SPIFFS false
#endif

#if USE_LITTLEFS
// Use LittleFS
#include "FS.h"

// The library has been merged into esp32 core release 1.0.6
#include <LITTLEFS.h> // https://github.com/lorol/LITTLEFS

FS *filesystem = &LITTLEFS;
#define FileFS LITTLEFS
#define FS_Name "LittleFS"
#elif USE_SPIFFS
#include <SPIFFS.h>
FS *filesystem = &SPIFFS;
#define FileFS SPIFFS
#define FS_Name "SPIFFS"
#else
// Use FFat
#include <FFat.h>
FS *filesystem = &FFat;
#define FileFS FFat
#define FS_Name "FFat"
#endif
//////

#define ESP_getChipId() ((uint32_t)ESP.getEfuseMac())

#define LED_BUILTIN 2
#define LED_ON HIGH
#define LED_OFF LOW

#else
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>

// From v1.1.0
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#define USE_LITTLEFS true

#if USE_LITTLEFS
#include <LittleFS.h>
FS *filesystem = &LittleFS;
#define FileFS LittleFS
#define FS_Name "LittleFS"
#else
FS *filesystem = &SPIFFS;
#define FileFS SPIFFS
#define FS_Name "SPIFFS"
#endif
//////

#define ESP_getChipId() (ESP.getChipId())

#define LED_ON LOW
#define LED_OFF HIGH
#endif

#include "Adafruit_MQTT.h"        //https://github.com/adafruit/Adafruit_MQTT_Library
#include "Adafruit_MQTT_Client.h" //https://github.com/adafruit/Adafruit_MQTT_Library

#if ESP32

//See file .../hardware/espressif/esp32/variants/(esp32|doitESP32devkitV1)/pins_arduino.h
#define LED_BUILTIN 10 // Pin D2 mapped to pin GPIO2/ADC12 of ESP32, control on-board LED
#define PIN_LED 2      // Pin D2 mapped to pin GPIO2/ADC12 of ESP32, control on-board LED
#endif

#if ESP32
const int BUTTON_PIN = 27;
#endif //ESP32

uint32_t timer = millis();

const char *CONFIG_FILE = "/ConfigMQTT.json";
// Indicates whether ESP has WiFi credentials saved from previous session
bool initialConfig = true; //default false

// Default configuration values for Adafruit IO MQTT

// Just dummy topics. To be updated later when got valid data from FS or Config Portal
String MQTT_Pub_Topic = "private/feeds/Temperature";
String MQTT_Topic_Start = "devices";

// Function Prototypes
void MQTT_connect();
bool readConfigFile();
bool writeConfigFile();

// For Config Portal
// SSID and PW for Config Portal
String ssid = "ESP_" + String(ESP_getChipId(), HEX);
String password;

// SSID and PW for your Router
String Router_SSID;
String Router_Pass;

#define FORMAT_FILESYSTEM false

#define MIN_AP_PASSWORD_SIZE 8

#define SSID_MAX_LEN 32
//From v1.0.10, WPA2 passwords can be up to 63 characters long.
#define PASS_MAX_LEN 64

typedef struct
{
  char wifi_ssid[SSID_MAX_LEN];
  char wifi_pw[PASS_MAX_LEN];
} WiFi_Credentials;

typedef struct
{
  String wifi_ssid;
  String wifi_pw;
} WiFi_Credentials_String;

#define NUM_WIFI_CREDENTIALS 2

// Assuming max 491 chars
#define TZNAME_MAX_LEN 50
#define TIMEZONE_MAX_LEN 50
typedef struct
{
  WiFi_Credentials WiFi_Creds[NUM_WIFI_CREDENTIALS];
  char TZ_Name[TZNAME_MAX_LEN]; // "America/Toronto"
  char TZ[TIMEZONE_MAX_LEN];    // "EST5EDT,M3.2.0,M11.1.0"
  uint16_t checksum;
} WM_Config;

WM_Config WM_config;

#define CONFIG_FILENAME F("/wifi_cred.dat")

#define USE_AVAILABLE_PAGES false
#define USE_STATIC_IP_CONFIG_IN_CP false
#define USE_ESP_WIFIMANAGER_NTP true
#define USING_AFRICA false
#define USING_AMERICA false
#define USING_ANTARCTICA false
#define USING_ASIA true
#define USING_ATLANTIC false
#define USING_AUSTRALIA false
#define USING_EUROPE false
#define USING_INDIAN false
#define USING_PACIFIC false
#define USING_ETC_GMT false
#define USE_CLOUDFLARE_NTP false
#define USING_CORS_FEATURE true
//////

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
#if defined(USE_DHCP_IP)
#undef USE_DHCP_IP
#endif
#define USE_DHCP_IP true
#else
// You can select DHCP or Static IP here
#define USE_DHCP_IP true
//#define USE_DHCP_IP     false
#endif

// Use USE_DHCP_IP == true for dynamic DHCP IP, false to use static IP which you have to change accordingly to your network
#if (defined(USE_STATIC_IP_CONFIG_IN_CP) && !USE_STATIC_IP_CONFIG_IN_CP)
// Force DHCP to be true
#if defined(USE_DHCP_IP)
#undef USE_DHCP_IP
#endif
#define USE_DHCP_IP true
#else
// You can select DHCP or Static IP here
#define USE_DHCP_IP true
//#define USE_DHCP_IP     false
#endif

#if (USE_DHCP_IP)
// Use DHCP
#warning Using DHCP IP
IPAddress stationIP = IPAddress(0, 0, 0, 0);
IPAddress gatewayIP = IPAddress(192, 168, 1, 1);
IPAddress netMask = IPAddress(255, 255, 255, 0);
#else
// Use static IP
#warning Using static IP
#ifdef ESP32
IPAddress stationIP = IPAddress(192, 168, 1, 232);
#else
IPAddress stationIP = IPAddress(192, 168, 2, 186);
#endif

IPAddress gatewayIP = IPAddress(192, 168, 1, 1);
IPAddress netMask = IPAddress(255, 255, 255, 0);
#endif

#define USE_CONFIGURABLE_DNS true

IPAddress dns1IP = gatewayIP;
IPAddress dns2IP = IPAddress(8, 8, 8, 8);

#define USE_CUSTOM_AP_IP false

// New in v1.4.0
IPAddress APStaticIP = IPAddress(192, 168, 100, 1);
IPAddress APStaticGW = IPAddress(192, 168, 100, 1);
IPAddress APStaticSN = IPAddress(255, 255, 255, 0);

#include <ESP_WiFiManager.h> //https://github.com/khoih-prog/ESP_WiFiManager

#define MQTT_SERVER_MAX_LEN 40
#define MQTT_SERVER_PORT_LEN 6
#define MQTT_USERNAME_LEN 100
#define MQTT_PASSWORD_LEN 40
char UUID[] = "1fca6912-f057-4e65-8997-eb67d3eeb4d5";
String Device_Name = "ESP32_Gate";
String Model = "HHGATE01";
String Des = "Gate device for counting In&Out";
char json_data[] = "{\"name\":\"ESP32_Gate\",\"model\":\"HHGATE01\",\"uuid\":\"1fca6912-f057-4e65-8997-eb67d3eeb4d5\",\"description\":\"Gate device for counting In&Out\"}";;
char mqtt_server[MQTT_SERVER_MAX_LEN] = "192.168.1.100";
char mqtt_port[MQTT_SERVER_PORT_LEN] = "1883";
char mqtt_username[MQTT_USERNAME_LEN];
char mqtt_password[MQTT_PASSWORD_LEN];
DynamicJsonDocument json_info(1024);
char strObj[500];
bool Firstrun = false;
//Button config
OneButton btn = OneButton(
    BUTTON_PIN, // Input pin for the button
    true,       // Button is active LOW
    true        // Enable internal pull-up resistor
);

// Create an ESP32 WiFiClient class to connect to the MQTT server
WiFiClient *client = NULL;

Adafruit_MQTT_Client *mqtt = NULL;
Adafruit_MQTT_Publish *Temperature = NULL;
Adafruit_MQTT_Publish *Start = NULL;

WiFi_AP_IPConfig WM_AP_IPconfig;
WiFi_STA_IPConfig WM_STA_IPconfig;

void initAPIPConfigStruct(WiFi_AP_IPConfig &in_WM_AP_IPconfig)
{
  in_WM_AP_IPconfig._ap_static_ip = APStaticIP;
  in_WM_AP_IPconfig._ap_static_gw = APStaticGW;
  in_WM_AP_IPconfig._ap_static_sn = APStaticSN;
}

void initSTAIPConfigStruct(WiFi_STA_IPConfig &in_WM_STA_IPconfig)
{
  in_WM_STA_IPconfig._sta_static_ip = stationIP;
  in_WM_STA_IPconfig._sta_static_gw = gatewayIP;
  in_WM_STA_IPconfig._sta_static_sn = netMask;
#if USE_CONFIGURABLE_DNS
  in_WM_STA_IPconfig._sta_static_dns1 = dns1IP;
  in_WM_STA_IPconfig._sta_static_dns2 = dns2IP;
#endif
}

void displayIPConfigStruct(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
  LOGERROR3(F("stationIP ="), in_WM_STA_IPconfig._sta_static_ip, ", gatewayIP =", in_WM_STA_IPconfig._sta_static_gw);
  LOGERROR1(F("netMask ="), in_WM_STA_IPconfig._sta_static_sn);
#if USE_CONFIGURABLE_DNS
  LOGERROR3(F("dns1IP ="), in_WM_STA_IPconfig._sta_static_dns1, ", dns2IP =", in_WM_STA_IPconfig._sta_static_dns2);
#endif
}

void configWiFi(WiFi_STA_IPConfig in_WM_STA_IPconfig)
{
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn, in_WM_STA_IPconfig._sta_static_dns1, in_WM_STA_IPconfig._sta_static_dns2);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  WiFi.config(in_WM_STA_IPconfig._sta_static_ip, in_WM_STA_IPconfig._sta_static_gw, in_WM_STA_IPconfig._sta_static_sn);
#endif
}

///////////////////////////////////////////

uint8_t connectMultiWiFi()
{
#if ESP32
  // For ESP32, this better be 0 to shorten the connect time.
  // For ESP32-S2/C3, must be > 500
#if (USING_ESP32_S2 || USING_ESP32_C3)
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS 500L
#else
  // For ESP32 core v1.0.6, must be >= 500
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS 800L
#endif
#else
  // For ESP8266, this better be 2200 to enable connect the 1st time
#define WIFI_MULTI_1ST_CONNECT_WAITING_MS 2200L
#endif

#define WIFI_MULTI_CONNECT_WAITING_MS 500L

  uint8_t status;

  WiFi.mode(WIFI_STA);

  LOGERROR(F("ConnectMultiWiFi with :"));

  if ((Router_SSID != "") && (Router_Pass != ""))
  {
    LOGERROR3(F("* Flash-stored Router_SSID = "), Router_SSID, F(", Router_Pass = "), Router_Pass);
    LOGERROR3(F("* Add SSID = "), Router_SSID, F(", PW = "), Router_Pass);
    wifiMulti.addAP(Router_SSID.c_str(), Router_Pass.c_str());
  }

  for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
  {
    // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
    if ((String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE))
    {
      LOGERROR3(F("* Additional SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw);
    }
  }

  LOGERROR(F("Connecting MultiWifi..."));

  //WiFi.mode(WIFI_STA);

#if !USE_DHCP_IP
  // New in v1.4.0
  configWiFi(WM_STA_IPconfig);
  //////
#endif

  int i = 0;
  status = wifiMulti.run();
  delay(WIFI_MULTI_1ST_CONNECT_WAITING_MS);

  while ((i++ < 20) && (status != WL_CONNECTED))
  {
    status = wifiMulti.run();

    if (status == WL_CONNECTED)
      break;
    else
      delay(WIFI_MULTI_CONNECT_WAITING_MS);
  }

  if (status == WL_CONNECTED)
  {
    LOGERROR1(F("WiFi connected after time: "), i);
    LOGERROR3(F("SSID:"), WiFi.SSID(), F(",RSSI="), WiFi.RSSI());
    LOGERROR3(F("Channel:"), WiFi.channel(), F(",IP address:"), WiFi.localIP());
  }
  else
  {
    LOGERROR(F("WiFi not connected"));

#if ESP8266
    ESP.reset();
#else
    ESP.restart();
#endif
  }

  return status;
}

void toggleLED()
{
  //toggle state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

#if USE_ESP_WIFIMANAGER_NTP

void printLocalTime()
{
#if ESP8266
  static time_t now;

  now = time(nullptr);

  if (now > 1451602800)
  {
    Serial.print("Local Date/Time: ");
    Serial.print(ctime(&now));
  }
#else
  struct tm timeinfo;

  getLocalTime(&timeinfo);

  // Valid only if year > 2000.
  // You can get from timeinfo : tm_year, tm_mon, tm_mday, tm_hour, tm_min, tm_sec
  if (timeinfo.tm_year > 100)
  {
    Serial.print("Local Date/Time: ");
    Serial.print(asctime(&timeinfo));
  }
#endif
}

#endif

void heartBeatPrint()
{
#if USE_ESP_WIFIMANAGER_NTP
  printLocalTime();
#else

  static int num = 1;

  if (WiFi.status() == WL_CONNECTED)
    Serial.print(F("W")); // W means connected to WiFi
  else
    Serial.print(F("N")); // N means not connected to WiFi

  if (num == 40)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 5 == 0)
  {
    Serial.print(F(" "));
  }
#endif
}

void publishMQTT()
{
  float some_number = 25.0 + (float)(millis() % 100) / 100;

  // For debug only
  //Serial.print(F("Published Temp = "));
  //Serial.println(some_number);

  MQTT_connect();

  if (Temperature->publish(some_number))
  {
    Serial.print(F("T")); // T means publishing OK
  }
  else
  {
    Serial.print(F("F")); // F means publishing failure
  }
  if (!Firstrun)
  {

    if (Start->publish(json_data))
    {
      Serial.print(F("send")); // T means publishing OK
    }
    else

    {
      Serial.print(F("F")); // F means publishing failure
    }
    Firstrun = true;
  }
}

void check_WiFi()
{
  if ((WiFi.status() != WL_CONNECTED))
  {
    Serial.println(F("\nWiFi lost. Call connectMultiWiFi in loop"));
    connectMultiWiFi();
  }
}

void check_status()
{
  static ulong checkstatus_timeout = 0;
  static ulong LEDstatus_timeout = 0;
  static ulong checkwifi_timeout = 0;
  static ulong mqtt_publish_timeout = 0;

  ulong current_millis = millis();

#define LED_INTERVAL 2000L
#define PUBLISH_INTERVAL 90000L

#define WIFICHECK_INTERVAL 1000L

#if USE_ESP_WIFIMANAGER_NTP
#define HEARTBEAT_INTERVAL 60000L
#else
#define HEARTBEAT_INTERVAL 10000L
#endif

  // Check WiFi every WIFICHECK_INTERVAL (1) seconds.
  if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
  {
    check_WiFi();
    checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
  }

  if ((current_millis > LEDstatus_timeout) || (LEDstatus_timeout == 0))
  {
    // Toggle LED at LED_INTERVAL = 2s
    toggleLED();
    LEDstatus_timeout = current_millis + LED_INTERVAL;
  }

  // Print hearbeat every HEARTBEAT_INTERVAL (10) seconds.
  if ((current_millis > checkstatus_timeout) || (checkstatus_timeout == 0))
  {
    heartBeatPrint();
    checkstatus_timeout = current_millis + HEARTBEAT_INTERVAL;
  }

  // Check every PUBLISH_INTERVAL (60) seconds.
  if ((current_millis > mqtt_publish_timeout) || (mqtt_publish_timeout == 0))
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      publishMQTT();
    }

    mqtt_publish_timeout = current_millis + PUBLISH_INTERVAL;
  }
}

int calcChecksum(uint8_t *address, uint16_t sizeToCalc)
{
  uint16_t checkSum = 0;

  for (uint16_t index = 0; index < sizeToCalc; index++)
  {
    checkSum += *(((byte *)address) + index);
  }

  return checkSum;
}

bool loadConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "r");
  LOGERROR(F("LoadWiFiCfgFile "));

  memset((void *)&WM_config, 0, sizeof(WM_config));

  // New in v1.4.0
  memset((void *)&WM_STA_IPconfig, 0, sizeof(WM_STA_IPconfig));
  //////

  if (file)
  {
    file.readBytes((char *)&WM_config, sizeof(WM_config));

    // New in v1.4.0
    file.readBytes((char *)&WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    LOGERROR(F("OK"));

    if (WM_config.checksum != calcChecksum((uint8_t *)&WM_config, sizeof(WM_config) - sizeof(WM_config.checksum)))
    {
      LOGERROR(F("WM_config checksum wrong"));

      return false;
    }

    // New in v1.4.0
    displayIPConfigStruct(WM_STA_IPconfig);
    //////

    return true;
  }
  else
  {
    LOGERROR(F("failed"));

    return false;
  }
}

void saveConfigData()
{
  File file = FileFS.open(CONFIG_FILENAME, "w");
  LOGERROR(F("SaveWiFiCfgFile "));

  if (file)
  {
    WM_config.checksum = calcChecksum((uint8_t *)&WM_config, sizeof(WM_config) - sizeof(WM_config.checksum));

    file.write((uint8_t *)&WM_config, sizeof(WM_config));

    displayIPConfigStruct(WM_STA_IPconfig);

    // New in v1.4.0
    file.write((uint8_t *)&WM_STA_IPconfig, sizeof(WM_STA_IPconfig));
    //////

    file.close();
    LOGERROR(F("OK"));
  }
  else
  {
    LOGERROR(F("failed"));
  }
}

void deleteOldInstances()
{
  // Delete previous instances
  if (mqtt)
  {
    delete mqtt;
    mqtt = NULL;

    Serial.println(F("Deleting old MQTT object"));
  }

  if (Temperature)
  {
    delete Temperature;
    Temperature = NULL;

    Serial.println(F("Deleting old Temperature object"));
  }
  if (Start)
  {
    delete Start;
    Start = NULL;

    Serial.println(F("Deleting old Start object"));
  }
}

void createNewInstances()
{
  if (!client)
  {
    client = new WiFiClient;

    Serial.print(F("\nCreating new WiFi client object : "));
    Serial.println(client ? F("OK") : F("failed"));
  }

  // Create new instances from new data
  if (!mqtt)
  {
    // Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
    mqtt = new Adafruit_MQTT_Client(client, mqtt_server, atoi(mqtt_port), mqtt_username, mqtt_password);

    Serial.print(F("Creating new MQTT object : "));

    if (mqtt)
    {
      Serial.println(F("OK"));
      Serial.println(String("SERVER = ") + mqtt_server + ", SERVERPORT = " + mqtt_port);
      Serial.println(String("USERNAME = ") + mqtt_username + ", KEY = " + mqtt_password);
    }
    else
      Serial.println(F("Failed"));
  }

  if (!Temperature)
  {
    Serial.print(F("Creating new MQTT_Pub_Topic,  Temperature = "));
    Serial.println(MQTT_Pub_Topic);

    Temperature = new Adafruit_MQTT_Publish(mqtt, MQTT_Pub_Topic.c_str());

    Serial.print(F("Creating new Temperature object : "));

    if (Temperature)
    {
      Serial.println(F("OK"));
      Serial.println(String("Temperature MQTT_Pub_Topic = ") + MQTT_Pub_Topic);
    }
    else
      Serial.println(F("Failed"));
  }

  if (!Start)
  {
    Serial.print(F("Creating new MQTT_Topic_Start,  start = "));
    Serial.println(MQTT_Topic_Start);

    Start = new Adafruit_MQTT_Publish(mqtt, MQTT_Topic_Start.c_str());

    Serial.print(F("Creating new Start object : "));

    if (Start)
    {
      Serial.println(F("OK"));
      Serial.println(String("Start MQTT_Pub_Topic = ") + MQTT_Topic_Start);
    }
    else
      Serial.println(F("Failed"));
  }
}

//event handler functions for button
static void handleClick()
{
  Serial.println(F("Button clicked!"));
  wifi_manager();
}

static void handleDoubleClick()
{
  Serial.println(F("Button double clicked!"));
}

static void handleLongPressStop()
{
  Serial.println(F("Button pressed for long time and then released!"));
  //newConfigData();
}

void wifi_manager()
{
  Serial.println(F("\nConfig Portal requested."));
  digitalWrite(LED_BUILTIN, LED_ON); 
  ESP_WiFiManager ESP_wifiManager("ConfigOnSwitchFS-MQTT");

  Serial.print(F("Opening Configuration Portal. "));

  Router_SSID = ESP_wifiManager.WiFi_SSID();
  Router_Pass = ESP_wifiManager.WiFi_Pass();

  if (!initialConfig && (Router_SSID != "") && (Router_Pass != ""))
  {
    ESP_wifiManager.setConfigPortalTimeout(120);
    Serial.println(F("Got stored Credentials. Timeout 120s"));
  }
  else{
    ESP_wifiManager.setConfigPortalTimeout(0);
    Serial.print(F("No timeout : "));
    if (initialConfig){
      Serial.println(F("DRD or No stored Credentials.."));
    }
    else{
      Serial.println(F("No stored Credentials."));
    }
  }
  String temp = "<p>Device Information<br><br><b>Device Name:</b> {a} <br><b>Model:</b> {b} <br><b>UUID:</b> {c} <br><b>Description:</b> {d} <br></p>";
  temp.replace("{a}", Device_Name);
  temp.replace("{b}", Model);
  temp.replace("{c}", UUID);
  temp.replace("{d}", Des);
  int str_len = temp.length() + 1;
  char char_array[str_len];
  temp.toCharArray(char_array, str_len);
  sprintf(mqtt_username, "GATE_%s", String(ESP_getChipId(), HEX));
  ESP_WMParameter custom_mqtt_server("MQTT Server", "mqtt_server", mqtt_server, MQTT_SERVER_MAX_LEN + 1);
  ESP_WMParameter custom_mqtt_port("MQTT Port", "mqtt_port", mqtt_port, MQTT_SERVER_PORT_LEN + 1);
  ESP_WMParameter custom_mqtt_username("Username", "username", mqtt_username, MQTT_USERNAME_LEN + 1, " readonly");
  ESP_WMParameter custom_mqtt_password("Password", "password", mqtt_password, MQTT_PASSWORD_LEN + 1, " readonly");
  ESP_WMParameter custom_text(char_array);

  ESP_wifiManager.addParameter(&custom_mqtt_server);
  ESP_wifiManager.addParameter(&custom_mqtt_port);
  ESP_wifiManager.addParameter(&custom_mqtt_username);
  ESP_wifiManager.addParameter(&custom_mqtt_password);
  ESP_wifiManager.addParameter(&custom_text);

  ESP_wifiManager.setMinimumSignalQuality(-1);
  ESP_wifiManager.setConfigPortalChannel(0);

#if USE_CUSTOM_AP_IP
  //set custom ip for portal
  // New in v1.4.0
  ESP_wifiManager.setAPStaticIPConfig(WM_AP_IPconfig);
  //////
#endif

#if !USE_DHCP_IP
#if USE_CONFIGURABLE_DNS
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5
  ESP_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask, dns1IP, dns2IP);
#else
  // Set static IP, Gateway, Subnetmask, Use auto DNS1 and DNS2.
  ESP_wifiManager.setSTAStaticIPConfig(stationIP, gatewayIP, netMask);
#endif
#endif

  // New from v1.1.1
#if USING_CORS_FEATURE
  ESP_wifiManager.setCORSHeader("Your Access-Control-Allow-Origin");
#endif

  // SSID to uppercase
  ssid.toUpperCase();
  password = "My" + ssid;

  // Start an access point
  // and goes into a blocking loop awaiting configuration.
  // Once the user leaves the portal with the exit button
  // processing will continue
  if (!ESP_wifiManager.startConfigPortal((const char *)ssid.c_str(), password.c_str()))
  {
    Serial.println(F("Not connected to WiFi but continuing anyway."));
  }
  else
  {
    // If you get here you have connected to the WiFi
    Serial.println(F("Connected...yeey :)"));
    Serial.print(F("Local IP: "));
    Serial.println(WiFi.localIP());
  }

  // Only clear then save data if CP entered and with new valid Credentials
  // No CP => stored getSSID() = ""
  if (String(ESP_wifiManager.getSSID(0)) != "")
  {
    // Stored  for later usage, from v1.1.0, but clear first
    memset(&WM_config, 0, sizeof(WM_config));

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      String tempSSID = ESP_wifiManager.getSSID(i);
      String tempPW = ESP_wifiManager.getPW(i);

      if (strlen(tempSSID.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_ssid, tempSSID.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_ssid) - 1);

      if (strlen(tempPW.c_str()) < sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1)
        strcpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str());
      else
        strncpy(WM_config.WiFi_Creds[i].wifi_pw, tempPW.c_str(), sizeof(WM_config.WiFi_Creds[i].wifi_pw) - 1);

      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ((String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE))
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw);
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
      }
    }

#if USE_ESP_WIFIMANAGER_NTP
    String tempTZ = ESP_wifiManager.getTimezoneName();

    if (strlen(tempTZ.c_str()) < sizeof(WM_config.TZ_Name) - 1)
      strcpy(WM_config.TZ_Name, tempTZ.c_str());
    else
      strncpy(WM_config.TZ_Name, tempTZ.c_str(), sizeof(WM_config.TZ_Name) - 1);

    const char *TZ_Result = ESP_wifiManager.getTZ(WM_config.TZ_Name);

    if (strlen(TZ_Result) < sizeof(WM_config.TZ) - 1)
      strcpy(WM_config.TZ, TZ_Result);
    else
      strncpy(WM_config.TZ, TZ_Result, sizeof(WM_config.TZ_Name) - 1);

    if (strlen(WM_config.TZ_Name) > 0)
    {
      LOGERROR3(F("Saving current TZ_Name ="), WM_config.TZ_Name, F(", TZ = "), WM_config.TZ);

#if ESP8266
      configTime(WM_config.TZ, "pool.ntp.org");
#else
      //configTzTime(WM_config.TZ, "pool.ntp.org" );
      //configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
#endif
    }
    else
    {
      LOGERROR(F("Current Timezone Name is not set. Enter Config Portal to set."));
    }
#endif

    // New in v1.4.0
    ESP_wifiManager.getSTAStaticIPConfig(WM_STA_IPconfig);
    //////

    saveConfigData();
  }

  // Getting posted form values and overriding local variables parameters
  // Config file is written regardless the connection state
  strncpy(mqtt_server, custom_mqtt_server.getValue(), sizeof(mqtt_server));
  strncpy(mqtt_port, custom_mqtt_port.getValue(), sizeof(mqtt_port));
  strncpy(mqtt_username, custom_mqtt_username.getValue(), sizeof(mqtt_username));
  strncpy(mqtt_password, custom_mqtt_password.getValue(), sizeof(mqtt_password));

  // Writing JSON config file to flash for next boot
  writeConfigFile();
  Serial.println("save");
  digitalWrite(LED_BUILTIN, LED_OFF); // Turn LED off as we are not in configuration mode.

  deleteOldInstances();

  MQTT_Pub_Topic = String(mqtt_username) + "/feeds/Temperature";
  createNewInstances();
}

bool readConfigFile()
{
  Serial.println("read mqtt file");
  // this opens the config file in read-mode
  File f = FileFS.open(CONFIG_FILE, "r");

  if (!f)
  {
    Serial.println(F("Config File not found"));
    return false;
  }
  else
  {
    // we could open the file
    size_t size = f.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size + 1]);

    // Read and store file contents in buf
    f.readBytes(buf.get(), size);
    // Closing file
    f.close();
    // Using dynamic JSON buffer which is not the recommended memory model, but anyway
    // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

#if (ARDUINOJSON_VERSION_MAJOR >= 6)

    DynamicJsonDocument json(1024);
    auto deserializeError = deserializeJson(json, buf.get());

    if (deserializeError)
    {
      Serial.println(F("JSON parseObject() failed"));
      return false;
    }

    serializeJson(json, Serial);

#else

    DynamicJsonBuffer jsonBuffer;
    // Parse JSON string
    JsonObject &json = jsonBuffer.parseObject(buf.get());

    // Test if parsing succeeds.
    if (!json.success())
    {
      Serial.println(F("JSON parseObject() failed"));
      return false;
    }

    json.printTo(Serial);

#endif
    Serial.println(F("OK"));

    if (json["mqtt_server"])
      strncpy(mqtt_server, json["mqtt_server"], sizeof(mqtt_server));
    if (json["mqtt_port"])
      strncpy(mqtt_port, json["mqtt_port"], sizeof(mqtt_port));
    if (json["mqtt_username"])
      strncpy(mqtt_username, json["mqtt_username"], sizeof(mqtt_username));
    if (json["mqtt_password"])
      strncpy(mqtt_password, json["mqtt_password"], sizeof(mqtt_password));

    // Parse all config file parameters, override
    // local config variables with parsed values
  }

  Serial.println(F("\nConfig File successfully parsed"));

  return true;
}

bool writeConfigFile()
{
  Serial.println(F("Saving Config File"));

#if (ARDUINOJSON_VERSION_MAJOR >= 6)
  DynamicJsonDocument json(1024);
#else
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
#endif

  // JSONify local configuration parameters
  json["mqtt_server"] = mqtt_server;
  json["mqtt_port"] = mqtt_port;
  json["mqtt_username"] = mqtt_username;
  json["mqtt_password"] = mqtt_password;

  // Open file for writing
  File f = FileFS.open(CONFIG_FILE, "w");

  if (!f)
  {
    Serial.println(F("Failed to open Config File for writing"));
    return false;
  }

#if (ARDUINOJSON_VERSION_MAJOR >= 6)
  serializeJsonPretty(json, Serial);
  // Write data to file and close it
  serializeJson(json, f);
#else
  json.prettyPrintTo(Serial);
  // Write data to file and close it
  json.printTo(f);
#endif

  f.close();

  Serial.println(F("\nConfig File successfully saved"));
  return true;
}

// this function is just to display newly saved data,
// it is not necessary though, because data is displayed
// after WiFi manager resets ESP32
void newConfigData()
{
  Serial.println();
  Serial.print(F("custom_AIO_SERVER: "));
}

void MQTT_connect()
{
  int8_t ret;

  MQTT_Pub_Topic = String(Device_Name) + "/feeds/Temperature";

  createNewInstances();

  // Return if already connected
  if (mqtt->connected())
  {
    return;
  }

  Serial.println(F("Connecting to WiFi MQTT (3 attempts)..."));

  uint8_t attempt = 3;

  while ((ret = mqtt->connect()) != 0)
  {
    // connect will return 0 for connected
    Serial.println(mqtt->connectErrorString(ret));
    Serial.println(F("Another attemtpt to connect to MQTT in 2 seconds..."));
    mqtt->disconnect();
    delay(2000); // wait 2 seconds
    attempt--;

    if (attempt == 0)
    {
      Serial.println(F("WiFi MQTT connection failed. Continuing with program..."));
      return;
    }
  }

  Serial.println(F("WiFi MQTT connection successful!"));
  //send
}

// Setup function
void setup()
{
  // Put your setup code here, to run once
  Serial.begin(115200);
  while (!Serial)
    ;

  delay(200);
  json_info["name"] = Device_Name;
  json_info["model"] = Model;
  json_info["uuid"] = UUID;
  json_info["description"] = Des;

  serializeJsonPretty(json_info, Serial);
  serializeJson(json_info, strObj);

  Serial.print(F("\nStarting ConfigOnSwichFS_MQTT_Ptr using "));
  Serial.print(FS_Name);
  Serial.print(F(" on "));
  Serial.println(ARDUINO_BOARD);
  Serial.println(ESP_WIFIMANAGER_VERSION);

  if (String(ESP_WIFIMANAGER_VERSION) < ESP_WIFIMANAGER_VERSION_MIN_TARGET)
  {
    Serial.print(F("Warning. Must use this example on Version equal or later than : "));
    Serial.println(ESP_WIFIMANAGER_VERSION_MIN_TARGET);
  }

  btn.attachClick(handleClick);
  btn.attachDoubleClick(handleDoubleClick);
  btn.attachLongPressStop(handleLongPressStop);

  // Initialize the LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Mount the filesystem
  if (FORMAT_FILESYSTEM)
  {
    Serial.println(F("Forced Formatting."));
    FileFS.format();
  }

  // Format FileFS if not yet
#ifdef ESP32
  if (!FileFS.begin(true))
#else
  if (!FileFS.begin())
#endif
  {
#ifdef ESP8266
    FileFS.format();
#endif

    Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));

    if (!FileFS.begin())
    {
      // prevents debug info from the library to hide err message.
      delay(100);

#if USE_LITTLEFS
      Serial.println(F("LittleFS failed!. Please use SPIFFS or EEPROM. Stay forever"));
#else
      Serial.println(F("SPIFFS failed!. Please use LittleFS or EEPROM. Stay forever"));
#endif

      while (true)
      {
        delay(1);
      }
    }
  }

  if (!readConfigFile())
  {
    Serial.println(F("Failed to read configuration file, using default values"));
  }

  // New in v1.4.0
  initAPIPConfigStruct(WM_AP_IPconfig);
  initSTAIPConfigStruct(WM_STA_IPconfig);
  //////

  if (!readConfigFile())
  {
    Serial.println(F("Can't read Config File, using default values"));
  }

  // Load stored data, the addAP ready for MultiWiFi reconnection
  bool configDataLoaded = loadConfigData();

  // Pretend CP is necessary as we have no AP Credentials
  initialConfig = true;

  if (configDataLoaded)
  {
#if USE_ESP_WIFIMANAGER_NTP
    if (strlen(WM_config.TZ_Name) > 0)
    {
      LOGERROR3(F("Current TZ_Name ="), WM_config.TZ_Name, F(", TZ = "), WM_config.TZ);

#if ESP8266
      configTime(WM_config.TZ, "pool.ntp.org");
#else
      //configTzTime(WM_config.TZ, "pool.ntp.org" );
      // configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
#endif
    }
    else
    {
      Serial.println(F("Current Timezone is not set. Enter Config Portal to set."));
    }
#endif

    for (uint8_t i = 0; i < NUM_WIFI_CREDENTIALS; i++)
    {
      // Don't permit NULL SSID and password len < MIN_AP_PASSWORD_SIZE (8)
      if ((String(WM_config.WiFi_Creds[i].wifi_ssid) != "") && (strlen(WM_config.WiFi_Creds[i].wifi_pw) >= MIN_AP_PASSWORD_SIZE))
      {
        LOGERROR3(F("* Add SSID = "), WM_config.WiFi_Creds[i].wifi_ssid, F(", PW = "), WM_config.WiFi_Creds[i].wifi_pw);
        wifiMulti.addAP(WM_config.WiFi_Creds[i].wifi_ssid, WM_config.WiFi_Creds[i].wifi_pw);
        initialConfig = false;
      }
    }
  }

  if (initialConfig)
  {
    Serial.println(F("Open Config Portal without Timeout: No stored WiFi Credentials"));

    wifi_manager();
  }
  else if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(F("ConnectMultiWiFi in setup"));

    connectMultiWiFi();
  }
  //configTzTime(WM_config.TZ, "time.nist.gov", "0.pool.ntp.org", "1.pool.ntp.org");
  digitalWrite(LED_BUILTIN, LED_OFF); // Turn led off as we are not in configuration mode.
  Serial.println("Starting");
}

// Loop function
void loop()
{
  // checking button state all the time
  btn.tick();

  // this is just for checking if we are connected to WiFi
  check_status();
}
