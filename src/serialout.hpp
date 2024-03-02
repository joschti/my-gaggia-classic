#ifndef _SERIALOUT_HPP_
#define _SERIALOUT_HPP_

#include "Arduino.h"
#include "mytask.hpp"

class SerialOut : public MyTask
{
public:
  SerialOut(uint32_t msUpdateRate) : MyTask(msUpdateRate)
  {
  }

  void taskHandler(uint32_t currMs, float temp)
  {
    if (this->timeHandler(currMs))
    {
      char buffer[70];
      sprintf(buffer, "temp %4.2f°C\n", temp);
      Serial.print(buffer);
    }
  }
};

#endif
