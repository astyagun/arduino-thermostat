#include "temperature_sensor.h"

void TemperatureSensor::setup() {
  sensors.begin();
  sensors.setWaitForConversion(false);
  sensors.getAddress(sensorAddress, 0);

  #ifdef DEBUG
    // Parasite power must be off to request temperature without blocking
    Serial.print("Parasite power is: ");
    if(sensors.isParasitePowerMode()) Serial.println("ON");
    else Serial.println("OFF");
    Serial.print("Sensor resolution: ");
    Serial.println(sensors.getResolution(sensorAddress), DEC);
    lastTemperatureReadAt = millis() - 1;
  #endif

  request();
}

float TemperatureSensor::measure() {
  unsigned long now                 = millis();
  unsigned long elapsedSinceRequest = now - lastTemperatureRequestedAt;

  if(elapsedSinceRequest >= TEMPERATURE_SENSOR_REQUEST_DELAY) {
    if(!request()) currentTemperature = TEMPERATURE_INVALID;
  }

  if(allowsRead) {
    now                 = millis();
    elapsedSinceRequest = now - lastTemperatureRequestedAt;
    if(elapsedSinceRequest > TEMPERATURE_SENSOR_READ_DELAY)
      currentTemperature = read();
  }

  return currentTemperature;
}

bool TemperatureSensor::request() {
  #ifdef DEBUG
    Serial.print("Requesting temperatures at ");
    Serial.print(millis(), DEC);
    Serial.println("...");
  #endif

  if(sensors.requestTemperaturesByAddress(sensorAddress)) {
    lastTemperatureRequestedAt = millis();
    allowsRead                 = true;

    #ifdef DEBUG
      Serial.print("Requested temperatures at ");
      Serial.print(lastTemperatureRequestedAt, DEC);
      Serial.println();
    #endif

    return true;
  } else {
    #ifdef DEBUG
      Serial.println("Failed requesting temperature by address! Is the sensor disconnected?");
    #endif

    return false;
  }
}

float TemperatureSensor::read() {
  #ifdef DEBUG
    Serial.print("Reading temperature at ");
    Serial.print(millis(), DEC);
    Serial.println("...");
  #endif

  float temperature = sensors.getTempC(sensorAddress);

  if(temperature != DEVICE_DISCONNECTED_C) {
    allowsRead = false;

    #ifdef DEBUG
      lastTemperatureReadAt = millis();
      Serial.print("Done reading at ");
      Serial.print(lastTemperatureReadAt, DEC);
      Serial.println();
      Serial.print("Temperature is: ");
      Serial.println(temperature);
    #endif
  } else {
    temperature = TEMPERATURE_INVALID;

    #ifdef DEBUG
      Serial.println("Error reading temperature by address! Is the sensor disconnected?");
    #endif
  }

  return temperature;
}
