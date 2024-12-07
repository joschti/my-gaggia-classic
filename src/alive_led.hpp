#ifndef _ALIVE_LED_HPP_
#define _ALIVE_LED_HPP_

#include "Arduino.h"
#include "mbed.h"
#include "mytask.hpp"

class AliveLed : public MyTask
{
public:
  AliveLed(PinName ledPin, uint32_t msUpdateRate) : MyTask(msUpdateRate), led(ledPin) {}

  void taskHandler()
  {
    if (this->timeHandler())
    {
      this->led = !this->led;
    }
  }

  void off(void)
  {
    this->led = false;
  }

  void on(void)
  {
    this->led = true;
  }

private:
  mbed::DigitalOut led;
};

#endif
