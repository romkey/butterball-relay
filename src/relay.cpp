#include <Arduino.h>

#include "relay.h"

#include "config.h"
#include "hw.h"

void relay_begin() {
 pinMode(RELAY_PIN, OUTPUT);
 pinMode(RELAY_PIN, INPUT_PULLUP);
 //digitalWrite(RELAY_PIN, LOW);
}


void relay_on() {
  digitalWrite(RELAY_PIN, HIGH);
}

void relay_off() {
  digitalWrite(RELAY_PIN, LOW);
}

void relay_check(int current_temp) {
  if(current_temp > RELAY_MAX_TEMP)
    relay_off();

  if(current_temp < RELAY_MAX_TEMP)
    relay_on();
}
