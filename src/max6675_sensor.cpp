#include "max6675_sensor.h"

// GPIO pin numbers
#ifdef ESP8266
#define CLK 14 // GPIO14 - D5 - SCK
#define CS 13  // GPIO13 - D7 - MOSI
#define D0 12  // GPIO12 - D6 - MISO
#endif

#ifdef ESP32
#define CLK 5  // GPIO5  - SS
#define CS 23  // GPIO23 - MOSI
#define D0 19  // GPIO19 - MISO
#endif

void MAX6675_Sensor::begin() {
  //  _max6675.begin(CLK, CS, D0);
  _max6675.begin(CLK, CS, D0);
}

void MAX6675_Sensor::handle() {
  if(millis() - _last_read > 1000) {
    _temperatureC = _max6675.readFahrenheit();
    _last_read = millis();
  }
}
