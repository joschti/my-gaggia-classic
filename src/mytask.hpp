#ifndef _MY_TASK_HPP_
#define _MY_TASK_HPP_

#include "Arduino.h"

class MyTask
{
public:
  MyTask(uint32_t msUpdateRate) : msUpdateRate(msUpdateRate)
  {
    this->prevMsUpdate = 0;
  }

  bool timeHandler(uint32_t currMs)
  {
    uint32_t msDiff = currMs - this->prevMsUpdate;
    if (msDiff > this->msUpdateRate)
    {
      this->prevMsUpdate = currMs;
      return true;
    }
    return false;
  }

private:
  uint32_t msUpdateRate;
  uint32_t prevMsUpdate;
};

#endif
