#include "controller.h"
#include "temperature_sensor.h"
#include "thermostat.h"

#define RELAY_PIN A2

#define LED_PIN 11
#define LED_BRIGHTNESS 1
#define LED_CYCLE_LENGTH 100

#define HIGH_TARGET_TEMPERATURE 22.0
#define LOW_TARGET_TEMPERATURE 10.0
#define TARGET_TEMPERATURE_HYSTERESIS 0.5

#define THERMOSTAT_ENABLED_EEPROM_ADDRESS 0
#define IS_TEMPERATURE_HIGH_EEPROM_ADDRESS 2

namespace Thermostat {
  namespace {
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

    bool enabled  = true;
    bool isTemperatureHigh  = true;
    bool heatingEnabled     = false;
    float targetTemperature = HIGH_TARGET_TEMPERATURE;

    JLed led(LED_PIN);

    void enabledDisabledCallback();
    void modeChangedCallback();
    void disableHeating();
    void enableHeating();
    void indicateCurrentState();
    void scheduleLedBlink(int);
    void updateLed();
    void updateTargetTemperature();
    void updateThermostatState();
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

    Controller::setup(enabledDisabledCallback, modeChangedCallback);

    pinMode(LED_PIN, OUTPUT);
    analogWrite(LED_PIN, LOW);

    TemperatureSensor::setup();

    enabled = EEPROM.read(THERMOSTAT_ENABLED_EEPROM_ADDRESS) == 1;
    isTemperatureHigh = EEPROM.read(IS_TEMPERATURE_HIGH_EEPROM_ADDRESS) == 1;
    updateThermostatState();
    updateTargetTemperature();

    indicateCurrentState();
  }

  void tick() {
    Controller::tick();
    updateLed();

    float temperature = TemperatureSensor::measure();

    if(enabled) {
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
    void indicateCurrentState() {
      if(isTemperatureHigh)
        scheduleLedBlink(1);
      else
        scheduleLedBlink(2);
    }

    void updateLed() {
      led.Update();
      if(!led.IsRunning()) led.Set(enabled ? LED_BRIGHTNESS : LOW);
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

    void enabledDisabledCallback() {
      enabled = !enabled;
      updateThermostatState();
      EEPROM.update(THERMOSTAT_ENABLED_EEPROM_ADDRESS, enabled ? 1 : 0);
    }

    void updateThermostatState() {
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
      if(isTemperatureHigh) {
        targetTemperature = HIGH_TARGET_TEMPERATURE;
        #ifdef DEBUG
          Serial.println("Termperature mode is HIGH");
        #endif
      } else {
        targetTemperature = LOW_TARGET_TEMPERATURE;
        #ifdef DEBUG
          Serial.println("Termperature mode is LOW");
        #endif
      }
    }

    void modeChangedCallback() {
      isTemperatureHigh = !isTemperatureHigh;

      EEPROM.update(IS_TEMPERATURE_HIGH_EEPROM_ADDRESS, isTemperatureHigh ? 1 : 0);
      updateTargetTemperature();
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
  };
};
