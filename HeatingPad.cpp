#include "Arduino.h"
#include "HeatingPad.h"

HeatingPad::HeatingPad(int gatePin, int tempPinA, int tempPinB) {
  this->gatePin = gatePin;
  pinMode(this->gatePin, OUTPUT);
  this->tempPins[0] = tempPinA;
  this->tempPins[1] = tempPinB;
}

/*
 * Turn on/off the heating pad according to the given target temperature
 */
void HeatingPad::adjustHeatingEdge() {
  float currentTemp = this->getTemp();

  if (!currentTemp || currentTemp > this->getTargetTemp()) {
    this->off();
  } else {
    this->on();
  }
}

void HeatingPad::on() {
  digitalWrite(this->gatePin, HIGH);
}

void HeatingPad::off() {
  digitalWrite(this->gatePin, LOW);
}

void HeatingPad::setTargetTemp(int temp) {
  this->targetTemp = temp;
}

int HeatingPad::getTargetTemp() {
  return this->targetTemp;
}

/*
 * Get a reliable temperature in celsius from the sensor(s)
 */
float HeatingPad::getTemp() {
  float maxTemp = 0;

  for(int i = 0; i < 2; i++) {
    float temp = this->getTemp(i);
    if (temp > maxTemp) {
      maxTemp = temp;
    }
  }

  return maxTemp;
}

/*
 * Get a reliable temperature in celsius from the sensor(s)
 */
float HeatingPad::getTemp(int pinIndex) {
  if (!this->tempPins[pinIndex]) {
    return 0;
  }
  
  float value = this->getRawTemp(pinIndex);

  if (!value || value < 10 || value > 70) {
    return 0;
  }
  
  return value;
}

/*
 * Get value of a temperature sensor in celsius
 */
float HeatingPad::getRawTemp(int pinIndex/* = 0*/) {
  PreciseLM35 sensor(this->tempPins[pinIndex], DEFAULT);
  return sensor.readCelsius();
}
