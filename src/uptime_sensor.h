#pragma once

#include "sensor.h"

class Uptime_Sensor : public Sensor {
 public:
  Uptime_Sensor(uint16_t update_frequency) : Sensor(update_frequency, 1, 1, true) { _start = millis(); };

  void begin() {};
  void handle() {};

  unsigned long uptime() { _mark_read(); return millis() - _start; }

 private:
  unsigned long _start;
};
