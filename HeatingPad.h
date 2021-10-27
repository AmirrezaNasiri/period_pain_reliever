// Stops loading this library twice
#ifndef PerilifHeatingPad_h
#define PerilifHeatingPad_h

// Basic Arduino headers
#include "Arduino.h"
#include <PreciseLM35.h>

class HeatingPad
{
  public:
    int *tempPins;
    
    HeatingPad(int gatePin, int tempPinA, int tempPinB);
    void adjustHeatingEdge();
    void on();
    void off();
    void setTargetTemp(int temp);
    int getTargetTemp();
    float getTemp();
    float getTemp(int pinIndex);
    float getRawTemp(int pinIndex = 0);

  private:
    int targetTemp = 0;
    int gatePin;
};

#endif
