#ifndef _MY_TASK_HPP_
#define _MY_TASK_HPP_

#include "Arduino.h"

class MyTask
{
public:
  MyTask(uint32_t msUpdateRate);
  bool timeHandler();
  uint32_t getMsUpdateRate();

private:
  // char[3] taskName;
  uint8_t taskId;
  uint32_t msUpdateRate;
  uint32_t prevMsUpdate;
};

#endif
