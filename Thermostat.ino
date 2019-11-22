#include <Arduino.h>
#include <Bounce2.h>
#include "temperature_sensor.h"

/* #define DEBUG_LOOP_LENGTH */

#define RELAY_PIN A2

#define BUTTON_PIN 12
#define BUTTON_DEBOUNCE_INTERVAL 25

#define LED_PIN 11
#define LED_BRIGHTNESS 1

#define TARGET_TEMPERATURE 24.0
#define TARGET_TEMPERATURE_HYSTERESIS 0.5

Bounce debouncer = Bounce();
TemperatureSensor temperature_sensor;
bool heating_enabled = false;
bool thermostat_enabled = true;

#ifdef DEBUG_LOOP_LENGTH
  #define MAX_LOOPS_TO_MEASURE 1500
  unsigned long lastLoopEndedAt = 0;
  byte loopLengths[MAX_LOOPS_TO_MEASURE];
  int loopNumber = 0;
#endif

void setup() {
  #if defined(DEBUG) || defined(DEBUG_LOOP_LENGTH)
    Serial.begin(9600);
    Serial.println("Setup...");
    Serial.print("Target temperature is ");
    Serial.print(TARGET_TEMPERATURE, DEC);
    Serial.print(", hysteresis is ");
    Serial.println(TARGET_TEMPERATURE_HYSTERESIS, DEC);
  #endif

  // Turn off Arduino UNO builtin LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  debouncer.attach(BUTTON_PIN);
  debouncer.interval(BUTTON_DEBOUNCE_INTERVAL);

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, thermostat_enabled);

  temperature_sensor.setup();
}

void loop() {
  float temperature = temperature_sensor.measure();

  debouncer.update();
  if(debouncer.fell()) {
    thermostat_enabled = !thermostat_enabled;
    if (thermostat_enabled) {
      analogWrite(LED_PIN, LED_BRIGHTNESS);
      #ifdef DEBUG
        Serial.println("Thermostat is ENABLED");
      #endif
    } else {
      digitalWrite(RELAY_PIN, LOW);
      heating_enabled = false;
      analogWrite(LED_PIN, LOW);
      #ifdef DEBUG
        Serial.println("Thermostat is DISABLED");
      #endif
    }
  }

  if(thermostat_enabled) {
    if(temperature == TEMPERATURE_INVALID) {
      digitalWrite(RELAY_PIN, LOW);
      heating_enabled = false;
      #ifdef DEBUG
        Serial.println("Temperature is unavailable");
        Serial.println("Heating is OFF");
      #endif
    } else {
      if(heating_enabled && temperature > TARGET_TEMPERATURE + TARGET_TEMPERATURE_HYSTERESIS) {
        digitalWrite(RELAY_PIN, LOW);
        heating_enabled = false;
        #ifdef DEBUG
          Serial.println("Heating is OFF");
        #endif
      }
      if(!heating_enabled && temperature < TARGET_TEMPERATURE - TARGET_TEMPERATURE_HYSTERESIS) {
        digitalWrite(RELAY_PIN, HIGH);
        heating_enabled = true;
        #ifdef DEBUG
          Serial.println("Heating is ON");
        #endif
      }
    }
  }

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
