#include <Arduino.h>

#include <Esp.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

// config.h contains private information and is not distributed with
// the project files. Look for config-example.h and edit it to set
// things like Wifi SSID, IFTTT API Keys, and MQTT and REST API
// information
#include "config.h"
#include "hw.h"

#include "relay.h"

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

#define MAC_ADDRESS_STR_LENGTH 6*2 + 5 + 1
static char mac_address_str[MAC_ADDRESS_STR_LENGTH];

#ifdef BUILD_INFO

#define STRINGIZE_NX(A) #A
#define STRINGIZE(A) STRINGIZE_NX(A)

static char build_info[] = STRINGIZE(BUILD_INFO);
#else
static char build_info[] = "not set";
#endif

static WiFiMulti wifiMulti;

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
  display.print("Wifi connecting...");
  display.display();

  wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
  wifiMulti.addAP(WIFI_SSID2, WIFI_PASSWORD2);
  wifiMulti.addAP(WIFI_SSID3, WIFI_PASSWORD3);

  WiFi.macAddress(mac_address);
  snprintf(hostname, sizeof(hostname), "%s-%02x%02x%02x", FURBALL_HOSTNAME, (int)mac_address[3], (int)mac_address[4], (int)mac_address[5]);
  Serial.printf("Hostname is %s\n", hostname);

  snprintf(mac_address_str, MAC_ADDRESS_STR_LENGTH, "%02x:%02x:%02x:%02x:%02x:%02x",
	   mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);

  WiFi.setHostname(hostname);

#if 0
  while(wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println();
  Serial.println("Connected!");

  if(!MDNS.begin(hostname))
    Serial.println("Error setting up MDNS responder!");
  else
    Serial.println("[mdns]");

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

  ArduinoOTA.begin();
  Serial.println("[ota]");
#endif

  display.clearDisplay();
  display.setCursor(0,0);
  display.println(hostname);
#if 0
  display.println(WiFi.localIP());
#endif
  display.display();

  bme280.begin();
  Serial.println("[bme280]");
  max6675.begin();
  Serial.println("[max6675]");

  relay_begin();
  relay_off();
  Serial.println("[relay]");

  delay(500);
}


void loop() {
  static unsigned long next_update_millis = 0;
  unsigned updates = 0;

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

    relay_check(max6675.temperatureC());
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  char buf[128];
  snprintf(buf, 128, "air  %d\ntcpl %d\nmaxtmp %d\nmintmp %d", (int)bme280.temperature(), (int)max6675.temperatureC(), (int)RELAY_MAX_TEMP, (int)RELAY_MIN_TEMP);
  display.print(buf);
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

  static bool first = true;
  if(first || (updates && (millis() > next_update_millis))) {
    next_update_millis = millis() + UPDATE_DELAY;

    IPAddress local = WiFi.status() == WL_CONNECTED ? WiFi.localIP() : IPAddress(0, 0, 0, 0);
    char buffer[500];

    if(first) {
      first = false;

      snprintf(buffer, 500, "{ \"id\": \"%s\", \"system\": { \"name\": \"%s\", \"build\": \"%s\", \"ip\": \"%d.%d.%d.%d\", \"rssi\": %d } }",
	       mac_address_str,
	       hostname, build_info, local[0], local[1], local[2], local[3], WiFi.RSSI());
    }

    snprintf(buffer, 500, "{ \"id\": \"%s\", \"system\": { \"name\": \"%s\", \"build\": \"%s\", \"freeheap\": %d, \"uptime\": %lu, \"ip\": \"%d.%d.%d.%d\", \"rssi\": %d, \"reboots\": %d, \"wifi_failures\": %d   }, \"environment\": { \"temperature\": %0.2f, \"humidity\": %0.2f, \"pressure\": %0.2f },   \"high_temperature\": %0.2f }",
	     mac_address_str,
	     hostname, build_info, ESP.getFreeHeap(), uptime.uptime()/1000, local[0], local[1], local[2], local[3], WiFi.RSSI(), bootCount, wifi_failures,
	     bme280.temperature(), bme280.humidity(), bme280.pressure(),
	     max6675.temperatureC());

#ifdef VERBOSE
    Serial.println(buffer);
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
