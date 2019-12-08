// DS18B20+ sensor library: https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <OneWire.h>
#include <DallasTemperature.h>

#include "common.h"

#define TEMPERATURE_INVALID -3.4028235E+38

namespace TemperatureSensor {
  void  setup();
  float measure();
};
