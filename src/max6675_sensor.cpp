#include "max6675_sensor.h"

#define D0 19
#define CS 23
#define CLK 5

void MAX6675_Sensor::begin() {
  //  _max6675.begin(CLK, CS, D0);
  _max6675.begin(CLK, CS, D0);

  Serial.println("MAX6675 inited");
}

void MAX6675_Sensor::handle() {
  _temperatureC = _max6675.readFahrenheit();
  Serial.print("____MAX temp ");
  Serial.println(_temperatureC);
}
