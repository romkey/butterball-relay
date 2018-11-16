#pragma once

#include "sensor.h"

class Freeheap_Sensor : public Sensor {
 public:
  Freeheap_Sensor(uint16_t update_frequency) : Sensor(update_frequency, 1, 1, true) { };

  void begin() {};
  void handle() {};

  unsigned long freeheap() { _mark_read(); return ESP.getFreeHeap(); }
};
