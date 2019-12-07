#include <Arduino.h>
#include <OneButton.h>
#include <EEPROM.h>
#include <JLed.h>
#include "temperature_sensor.h"
#include "common.h"

/* #define DEBUG_LOOP_LENGTH */

#define RELAY_PIN A2

#define BUTTON_PIN 12

#define LED_PIN 11
#define LED_BRIGHTNESS 1
#define LED_CYCLE_LENGTH 100

#define HIGH_TARGET_TEMPERATURE 22.0
#define LOW_TARGET_TEMPERATURE 10.0
#define TARGET_TEMPERATURE_HYSTERESIS 0.5

#define THERMOSTAT_ENABLED_EEPROM_ADDRESS 0
#define IS_TEMPERATURE_HIGH_EEPROM_ADDRESS 2

class CustomBlinkBrightnessEvaluator : public jled::BrightnessEvaluator {
  public:
    uint8_t Eval(uint32_t t) const override {
      if(t < LED_CYCLE_LENGTH)
        return LED_BRIGHTNESS * 2;
      else
        return LOW;
    }
    uint16_t Period() const override { return LED_CYCLE_LENGTH * 2; }
};
CustomBlinkBrightnessEvaluator customBlink;

bool thermostatEnabled = true;
bool heatingEnabled = false;
bool isTemperatureHigh = true;
float targetTemperature = HIGH_TARGET_TEMPERATURE;

OneButton button(BUTTON_PIN, true);
auto led = JLed(LED_PIN);
TemperatureSensor temperature_sensor;

void buttonClicked();
void buttonLongPressed();
void disableHeating();
void enableHeating();
void indicateCurrentState();
void scheduleLedBlink(int);
void updateLed();

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
    Serial.print("Target temperature is ");
    Serial.print(targetTemperature, DEC);
    Serial.print(", hysteresis is ");
    Serial.println(TARGET_TEMPERATURE_HYSTERESIS, DEC);
  #endif

  // Turn off Arduino UNO builtin LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  button.attachClick(buttonClicked);
  button.attachLongPressStart(buttonLongPressed);

  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, LOW);

  temperature_sensor.setup();

  thermostatEnabled = EEPROM.read(THERMOSTAT_ENABLED_EEPROM_ADDRESS) == 1;
  isTemperatureHigh = EEPROM.read(IS_TEMPERATURE_HIGH_EEPROM_ADDRESS) == 1;

  indicateCurrentState();
}

void loop() {
  button.tick();
  updateLed();

  float temperature = temperature_sensor.measure();

  if(thermostatEnabled) {
    if(temperature == TEMPERATURE_INVALID) {
      disableHeating();
      #ifdef DEBUG
        Serial.println("Temperature is unavailable");
        Serial.println("Heating is OFF");
      #endif
    } else {
      if(heatingEnabled && temperature > targetTemperature + TARGET_TEMPERATURE_HYSTERESIS) {
        disableHeating();
        #ifdef DEBUG
          Serial.println("Heating is OFF");
        #endif
      }
      if(!heatingEnabled && temperature < targetTemperature - TARGET_TEMPERATURE_HYSTERESIS) {
        enableHeating();
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

void indicateCurrentState() {
  if(isTemperatureHigh)
    scheduleLedBlink(1);
  else
    scheduleLedBlink(2);
}

void updateLed() {
  led.Update();
  if(!led.IsRunning()) led.Set(thermostatEnabled ? LED_BRIGHTNESS : LOW);
}

void scheduleLedBlink(int times) {
  led.Stop().Off().Update();
  led
    .Reset()
    .DelayBefore(LED_CYCLE_LENGTH * 5)
    .UserFunc(&customBlink)
    .Repeat(times)
    .DelayAfter(LED_CYCLE_LENGTH * 5);
}

void buttonClicked() {
  thermostatEnabled = !thermostatEnabled;

  EEPROM.update(THERMOSTAT_ENABLED_EEPROM_ADDRESS, thermostatEnabled ? 1 : 0);

  if(thermostatEnabled) {
    #ifdef DEBUG
      Serial.println("Thermostat is ENABLED");
    #endif
  } else {
    disableHeating();
    #ifdef DEBUG
      Serial.println("Thermostat is DISABLED");
    #endif
  }
}

void buttonLongPressed() {
  isTemperatureHigh = !isTemperatureHigh;

  EEPROM.update(IS_TEMPERATURE_HIGH_EEPROM_ADDRESS, isTemperatureHigh ? 1 : 0);

  if(isTemperatureHigh) {
    targetTemperature = HIGH_TARGET_TEMPERATURE;
    #ifdef DEBUG
      Serial.println("Termperature is HIGH");
    #endif
  } else {
    targetTemperature = LOW_TARGET_TEMPERATURE;
    #ifdef DEBUG
      Serial.println("Termperature is LOW");
    #endif
  }

  indicateCurrentState();
}

void enableHeating() {
  heatingEnabled = true;
  digitalWrite(RELAY_PIN, HIGH);
}

void disableHeating() {
  heatingEnabled = false;
  digitalWrite(RELAY_PIN, LOW);
}
