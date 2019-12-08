#include <Arduino.h>

#include "common.h"
#include "thermostat.h"

/* #define DEBUG_LOOP_LENGTH */

#ifdef DEBUG_LOOP_LENGTH
  #define MAX_LOOPS_TO_MEASURE 1000
  unsigned long lastLoopEndedAt = 0;
  byte loopLengths[MAX_LOOPS_TO_MEASURE];
  int loopNumber = 0;
#endif

void setup() {
  #if defined(DEBUG) || defined(DEBUG_LOOP_LENGTH)
    Serial.begin(9600);
    Serial.println("Setup...");
  #endif

  // Turn off Arduino UNO builtin LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Thermostat::setup();
}

void loop() {
  Thermostat::tick();

  #ifdef DEBUG_LOOP_LENGTH
    if(loopNumber <= MAX_LOOPS_TO_MEASURE) {
      unsigned long now = millis();
      if(now - lastLoopEndedAt > 1) {
        loopLengths[loopNumber] = now - lastLoopEndedAt;
        loopNumber++;
      }
      lastLoopEndedAt = now;
    }
    if(loopNumber == MAX_LOOPS_TO_MEASURE + 1) {
      Serial.print("Loop lengths:\r\n{");
      for(int i = 0; i < MAX_LOOPS_TO_MEASURE; i++) {
        Serial.print(loopLengths[i], DEC);
        Serial.print(", ");
      }
      Serial.println("}");
      loopNumber++;
    }
  #endif
}
