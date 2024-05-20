#ifndef _SERIALOUT_HPP_
#define _SERIALOUT_HPP_

#ifdef SERIALOUT_ENABLE

#include "Arduino.h"
#include "mytask.hpp"

class SerialOut : public MyTask
{
public:
  SerialOut(uint32_t msUpdateRate) : MyTask(msUpdateRate)
  {
  }

  void taskHandler(uint32_t currMs, float temp, bool heaterActive)
  {
    if (this->timeHandler(currMs))
    {
      char buffer[70];
      sprintf(buffer, "%4.2f,%d\n", temp, heaterActive);
      Serial.print(buffer);
    }
  }
};

#endif

#endif
