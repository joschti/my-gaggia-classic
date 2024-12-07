#include "mytask.hpp"

static uint8_t numInstances = 0;

MyTask::MyTask(uint32_t msUpdateRate) : msUpdateRate(msUpdateRate)
{
  // every task receives an ID depending on the order of initialization
  this->taskId = numInstances;
  numInstances++;

  // spread the load by using the task ID to determine the first execution
  this->prevMsUpdate = 7 * this->taskId;
}

bool MyTask::timeHandler()
{
  uint32_t currMs = millis();
  uint32_t msDiff = currMs - this->prevMsUpdate;
  if (msDiff > this->msUpdateRate)
  {
#ifdef SERIALOUT_ENABLE
    char buffer[40];
    sprintf(buffer, "taskId%d rtPerf%ld\n", this->taskId, (msDiff - this->msUpdateRate));
    Serial.print(buffer);
#endif
    this->prevMsUpdate = currMs;
    return true;
  }
  return false;
}

uint32_t MyTask::getMsUpdateRate()
{
  return this->msUpdateRate;
}
