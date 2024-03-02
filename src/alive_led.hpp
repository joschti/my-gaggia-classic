#ifndef _ALIVE_LED_HPP_
#define _ALIVE_LED_HPP_

#include "Arduino.h"
#include "mbed.h"
#include "mytask.hpp"

class AliveLed : public MyTask
{
public:
  AliveLed(PinName ledPin, uint32_t msUpdateRate) : MyTask(msUpdateRate), led(ledPin) {}

  void taskHandler(uint32_t currMs)
  {
    if (this->timeHandler(currMs))
    {
      this->led = !this->led;
    }
  }

private:
  mbed::DigitalOut led;
};

#endif
