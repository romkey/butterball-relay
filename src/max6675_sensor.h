#pragma once

#include "sensor.h"

#include <max6675.h>

class MAX6675_Sensor : public Sensor {
 public:
  MAX6675_Sensor(uint16_t update_frequency, uint16_t accuracy, uint16_t precision, boolean calibrated) : Sensor(update_frequency, accuracy, precision, calibrated) {};

  void begin();
  void handle();

  float temperatureC() { _mark_read(); return _temperatureC; };
  float temperatureF() { float tempC = temperatureC(); return (tempC*9/5) + 32; }

 private:
  MAX6675 _max6675;

  float _temperatureC;
  float _humidity;
  float _pressure;
  float _altitude;
};
