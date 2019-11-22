#include <Arduino.h>

#include <ESP.h>

#ifdef ESP32
#include <ESPmDNS.h>
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#endif


// config.h contains private information and is not distributed with
// the project files. Look for config-example.h and edit it to set
// things like Wifi SSID, IFTTT API Keys, and MQTT and REST API
// information
#include "config.h"
#include "hw.h"

#include <ArduinoOTA.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display;

#include "bme280_sensor.h"
#include "max6675_sensor.h"
#include "uptime_sensor.h"
#include "freeheap_sensor.h"

BME280_Sensor bme280(UPDATE_DELAY, 0, 0, false);
MAX6675_Sensor max6675(UPDATE_DELAY, 0, 0, false);
Uptime_Sensor uptime(UPDATE_DELAY);
Freeheap_Sensor freeheap(UPDATE_DELAY);

#include <PubSubClient.h>

#include <PubSubClient.h>
static WiFiClient wifi_mqtt_client;
static PubSubClient mqtt_client(wifi_mqtt_client);

#ifdef IFTTT_API_KEY
#include <IFTTTWebhook.h>
IFTTTWebhook ifttt(IFTTT_API_KEY, IFTTT_EVENT_NAME);
#endif


#ifdef IFTTT_API_KEY

#ifdef ESP32
#include <rom/rtc.h>

const char* reboot_reason(int code) {
  switch(code) {
    case 1 : return "POWERON_RESET";          /**<1, Vbat power on reset*/
    case 3 : return "SW_RESET";               /**<3, Software reset digital core*/
    case 4 : return "OWDT_RESET";             /**<4, Legacy watch dog reset digital core*/
    case 5 : return "DEEPSLEEP_RESET";        /**<5, Deep Sleep reset digital core*/
    case 6 : return "SDIO_RESET";             /**<6, Reset by SLC module, reset digital core*/
    case 7 : return "TG0WDT_SYS_RESET";       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : return "TG1WDT_SYS_RESET";       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : return "RTCWDT_SYS_RESET";       /**<9, RTC Watch dog Reset digital core*/
    case 10 : return "INTRUSION_RESET";       /**<10, Instrusion tested to reset CPU*/
    case 11 : return "TGWDT_CPU_RESET";       /**<11, Time Group reset CPU*/
    case 12 : return "SW_CPU_RESET";          /**<12, Software reset CPU*/
    case 13 : return "RTCWDT_CPU_RESET";      /**<13, RTC Watch dog Reset CPU*/
    case 14 : return "EXT_CPU_RESET";         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return "RTCWDT_BROWN_OUT_RESET";/**<15, Reset when the vdd voltage is not stable*/
    case 16 : return "RTCWDT_RTC_RESET";      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : return "NO_MEAN";
  }
}
#endif

#endif

#define MAC_ADDRESS_STR_LENGTH 6*2 + 5 + 1
static char mac_address_str[MAC_ADDRESS_STR_LENGTH];
  
#ifdef BUILD_INFO

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

static char build_info[] = STRINGIZE(BUILD_INFO);
#else
static char build_info[] = "not set";
#endif

static RTC_DATA_ATTR int bootCount = 0;
static RTC_DATA_ATTR int wifi_failures = 0;

static char hostname[sizeof(FURBALL_HOSTNAME) + 8];

void setup() {
  byte mac_address[6];

  bootCount++;

  delay(500);

  Serial.begin(115200);
  Serial.println("Hello World!");
  Serial.printf("Build: %s\n", build_info);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Hello, world!");
  display.print("Wifi ");
  display.println(WIFI_SSID);
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  WiFi.macAddress(mac_address);
  snprintf(hostname, sizeof(hostname), "%s-%02x%02x%02x", FURBALL_HOSTNAME, (int)mac_address[3], (int)mac_address[4], (int)mac_address[5]);
  Serial.printf("Hostname is %s\n", hostname);

  snprintf(mac_address_str, MAC_ADDRESS_STR_LENGTH, "%02x:%02x:%02x:%02x:%02x:%02x",
	   mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

#ifdef ESP32
  WiFi.setHostname(hostname);
#endif

  while(!WiFi.isConnected()) {
    Serial.print(".");
    delay(100);
  }

  Serial.println();
  Serial.println("Connected!");

#ifdef IFTTT_API_KEY
#ifdef ESP32
  ifttt.trigger("reboot", reboot_reason(rtc_get_reset_reason(0)),  reboot_reason(rtc_get_reset_reason(1)));
#else
  ifttt.trigger("reboot");
#endif
#endif

#ifdef ESP32  
  if(!MDNS.begin(hostname))
    Serial.println("Error setting up MDNS responder!");
  else
    Serial.println("[mdns]");
#endif

  mqtt_client.setServer(MQTT_HOST, MQTT_PORT);
  mqtt_client.connect(MQTT_UUID, MQTT_USER, MQTT_PASS);
  Serial.println("[mqtt]");

#ifdef ESP32
  ArduinoOTA.setHostname(hostname);
   ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
      ESP.restart();
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
#endif
  ArduinoOTA.begin();
  Serial.println("[ota]");

  display.clearDisplay();
  display.setCursor(0,0);
  display.println(hostname);
  display.println(WiFi.localIP());
  display.display();

  pinMode(VBAT_ENABLE, OUTPUT);
  Serial.println("[battery]");

  bme280.begin();
  Serial.println("[bme280]");
  max6675.begin();
  Serial.println("[max6675]");

  delay(500);
}


void loop() {
  static unsigned long next_update_millis = 0;
  static unsigned long last_mqtt_check = 0;
  unsigned updates = 0;

  mqtt_client.loop();

  if(millis() > last_mqtt_check + 5000) {
    if(!mqtt_client.connected()) {
      mqtt_client.connect(MQTT_UUID, MQTT_USER, MQTT_PASS);
      Serial.println("mqtt reconnect");
    }

    last_mqtt_check = millis();
  }

  ArduinoOTA.handle();

  bme280.handle();
  max6675.handle();

  if(bme280.ready_for_update()) {
    updates++;

#ifdef VERBOSE
    Serial.printf("Ambient temperature %.2f\n", bme280.temperature());
    Serial.printf("Ambient pressure %.2f\n", bme280.pressure());
    Serial.printf("Ambient humidity %.2f\n", bme280.humidity());
#endif
  }

  if(max6675.ready_for_update()) {
    updates++;

#ifdef VERBOSE
    Serial.printf("Hightemp %0.2f\n", max6675.temperatureC());
#endif
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(4);
  display.println((int)max6675.temperatureC());
  display.display();

  if(uptime.ready_for_update()) {
    updates++;

#ifdef VERBOSE
    Serial.printf("Uptime %.2f seconds\n", uptime.uptime()/1000.0);
#endif
  }

  if(freeheap.ready_for_update()) {
    updates++;

#ifdef VERBOSE
    Serial.printf("Freeheap %lu bytes\n", freeheap.freeheap());
#endif
  }

  // battery
  // halved by voltage divider so scale 0 - 4096 to be 0 to 6.6
  
  digitalWrite(VBAT_ENABLE, LOW);
  delay(100);

  uint16_t battery_raw = analogRead(VBAT_READ);
  unsigned battery_voltage = map(battery_raw, 0, 4095, 0, 66);
  pinMode(VBAT_ENABLE, INPUT);

  static bool first = true;
  if(first || (updates && (millis() > next_update_millis))) {
    next_update_millis = millis() + UPDATE_DELAY;

    IPAddress local = WiFi.localIP();
    char buffer[500];

    if(first) {
      first = false;

      snprintf(buffer, 500, "{ \"id\": \"%s\", \"system\": { \"name\": \"%s\", \"build\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"rssi\": %d } }",
	       MQTT_UUID,
	       hostname, build_info, local[0], local[1], local[2], local[3], WiFi.RSSI());
      mqtt_client.publish("/butterball/log", buffer);
    }

    snprintf(buffer, 500, "{ \"id\": \"%s\", \"system\": { \"name\": \"%s\", \"build\": \"%s\", \"freeheap\": %d, \"uptime\": %lu, \"ip\": \"%d.%d.%d.%d\", \"rssi\": %d, \"reboots\": %d, \"wifi_failures\": %d,  \"battery_voltage\": %0.1f, \"battery_raw\": %u }, \"environment\": { \"temperature\": %0.2f, \"humidity\": %0.2f, \"pressure\": %0.2f },   \"high_temperature\": %0.2f }",
	     MQTT_UUID,
	     hostname, build_info, ESP.getFreeHeap(), uptime.uptime()/1000, local[0], local[1], local[2], local[3], WiFi.RSSI(), bootCount, wifi_failures, battery_voltage, battery_raw,
	     bme280.temperature(), bme280.humidity(), bme280.pressure(),
	     max6675.temperatureC());

#ifdef VERBOSE
    Serial.println(buffer);
#endif

    if(max6675.temperatureC() < LOWEST_TEMPERATURE)
      return;

#ifdef MQTT_HOST
    mqtt_client.publish("/butterball", buffer);
#endif

#ifdef REST_API_ENDPOINT
    void post(char *json);
    post(buffer);
#endif
  }
}

#ifdef REST_API_ENDPOINT
void post(char *json) {
  HTTPClient http;

  http.begin(String(REST_API_ENDPOINT));
  http.addHeader("Content-Type", "application/json");
  int response = http.POST(json);

#ifdef VERBOSE
  if(response > 0) {
    Serial.printf("HTTP status code %d\n", response);
  } else {
    Serial.printf("HTTPClient error %d\n", response);
  }
#endif

  http.end();
}
#endif
