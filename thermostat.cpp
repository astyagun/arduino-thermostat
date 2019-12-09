#include "controller.h"
#include "temperature_sensor.h"
#include "thermostat.h"

#define RELAY_PIN A2

#define STATE_LED_PIN 11
#define STATE_LED_BRIGHTNESS 1
#define MODE_LED_PIN 7

#define HIGH_TARGET_TEMPERATURE 22.0
#define LOW_TARGET_TEMPERATURE 10.0
#define TARGET_TEMPERATURE_HYSTERESIS 0.5

#define THERMOSTAT_ENABLED_EEPROM_ADDRESS 0
#define LOW_TEMPERATURE_MODE_ENABLED_EEPROM_ADDRESS 2

namespace Thermostat {
  namespace {
    bool enabled                   = true;
    bool lowTemperatureModeEnabled = false;
    bool heatingEnabled            = false;
    float targetTemperature        = HIGH_TARGET_TEMPERATURE;

    void disableHeating();
    void enableHeating();
    void modeChangedCallback();
    void scheduleLedBlink(int);
    void stateChangedCallback();
    void updateLed();
    void updateState();
    void updateTargetTemperature();
  }

  void setup() {
    #ifdef DEBUG
      Serial.print("Target temperature is ");
      Serial.print(targetTemperature, DEC);
      Serial.print(", hysteresis is ");
      Serial.println(TARGET_TEMPERATURE_HYSTERESIS, DEC);
    #endif

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    Controller::setup(stateChangedCallback, modeChangedCallback);

    pinMode(STATE_LED_PIN, OUTPUT);
    pinMode(MODE_LED_PIN, OUTPUT);

    TemperatureSensor::setup();

    enabled = EEPROM.read(THERMOSTAT_ENABLED_EEPROM_ADDRESS) == 1;
    lowTemperatureModeEnabled = EEPROM.read(LOW_TEMPERATURE_MODE_ENABLED_EEPROM_ADDRESS) == 1;
    updateState();
    updateTargetTemperature();
  }

  void tick() {
    Controller::tick();
    updateLed();

    if(enabled) {
      float temperature = TemperatureSensor::measure();

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
  }

  namespace {
    void updateLed() {
      analogWrite(STATE_LED_PIN, enabled ? STATE_LED_BRIGHTNESS : 0);
      digitalWrite(MODE_LED_PIN, lowTemperatureModeEnabled ? HIGH : LOW);
    }

    void stateChangedCallback() {
      enabled = !enabled;
      updateState();
      EEPROM.update(THERMOSTAT_ENABLED_EEPROM_ADDRESS, enabled ? 1 : 0);
    }

    void updateState() {
      if(enabled) {
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

    void updateTargetTemperature() {
      if(lowTemperatureModeEnabled) {
        targetTemperature = LOW_TARGET_TEMPERATURE;
        #ifdef DEBUG
          Serial.println("Low temperature mode is ENABLED");
        #endif
      } else {
        targetTemperature = HIGH_TARGET_TEMPERATURE;
        #ifdef DEBUG
          Serial.println("Low temperature mode is DISABLED");
        #endif
      }
    }

    void modeChangedCallback() {
      lowTemperatureModeEnabled = !lowTemperatureModeEnabled;

      EEPROM.update(LOW_TEMPERATURE_MODE_ENABLED_EEPROM_ADDRESS, lowTemperatureModeEnabled ? 1 : 0);
      updateTargetTemperature();
    }

    void enableHeating() {
      heatingEnabled = true;
      digitalWrite(RELAY_PIN, HIGH);
    }

    void disableHeating() {
      heatingEnabled = false;
      digitalWrite(RELAY_PIN, LOW);
    }
  };
};
