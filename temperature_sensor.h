// DS18B20+ sensor library: https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <OneWire.h>
#include <DallasTemperature.h>

/* #define DEBUG */

#define TEMPERATURE_SENSOR_PIN 2
#define TEMPERATURE_SENSOR_READ_DELAY 750
#define TEMPERATURE_SENSOR_REQUEST_DELAY 60000

#define TEMPERATURE_INVALID -3.4028235E+38

class TemperatureSensor {
  public:
    void  setup();
    float measure();
  private:
    OneWire           oneWire{TEMPERATURE_SENSOR_PIN};
    DallasTemperature sensors{&oneWire};
    DeviceAddress     sensorAddress;
    unsigned long     lastTemperatureRequestedAt;
    bool              allowsRead         = false;
    float             currentTemperature = TEMPERATURE_INVALID;

    #ifdef DEBUG
      unsigned long lastTemperatureReadAt;
    #endif

    bool  request();
    float read();
};
