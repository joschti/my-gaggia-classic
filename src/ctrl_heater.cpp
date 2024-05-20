#include "ctrl_heater.hpp"

#define CTRL_HEATER_DAC_RES 5 //! equivalent to 5 bits DAC resolution
#define CTRL_HEATER_DAC_STEP_SIZE 1
#define CTRL_HEATER_DAC_NUM_STEPS (1 << CTRL_HEATER_DAC_RES)
#define CTRL_HEATER_DAC_MAX_VALUE (CTRL_HEATER_DAC_NUM_STEPS - 1)
#define CTRL_HEATER_DUTY_CYCLE_INTERVAL (CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS * CTRL_HEATER_DAC_NUM_STEPS) //! (20 * 32) = 640 ms

ControlHeater::ControlHeater(PinName activateHeaterPin, ctrl_heater_t type)
    : MyTask(CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS), // ensure we use max SSR frequency
      activateHeater(activateHeaterPin),
      type(type)
{
}

void ControlHeater::taskHandler(uint32_t currMs, float temp)
{
  if (this->timeHandler(currMs))
  {
    switch (this->type)
    {
    case CTRL_HEATER_TWO_POINT:
      ctrl_two_point(temp);
      break;
    case CTRL_HEATER_P:
      ctrl_proportional(temp);
      break;
    case CTRL_HEATER_PID:
      // not implemented yet
      break;
    case CTRL_HEATER_KALMAN:
      // not implemented yet
      break;
    }
  }
}

bool ControlHeater::isHeaterActive()
{
  return this->activateHeater ? true : false;
}

//! 2-point temperature controller implementation
void ControlHeater::ctrl_two_point(float temp)
{
  const float temp_low = 82.0f;
  const float temp_high = 83.0f;
  if (temp > temp_high)
  {
    // switch off heater
    this->activateHeater = false;
  }
  else if (temp < temp_low)
  {
    // switch on heater
    this->activateHeater = true;
  }
}

//! P temperature controller implementation
void ControlHeater::ctrl_proportional(float temp)
{
  ////////////////////////////////////////////////////////////////
  // controller parameterization
  ////////////////////////////////////////////////////////////////
  const float tempRef = 50.0f;
  const float kp = 1.0f;

  ////////////////////////////////////////////////////////////////
  // static memory
  ////////////////////////////////////////////////////////////////
  static uint16_t onDurationTime = 0;
  // every CTRL_HEATER_DUTY_CYCLE_INTERVAL, actual duty cycle is recalculated and updated
  static int16_t nextDutyCycleCalc = 0;

  ////////////////////////////////////////////////////////////////
  // controller update
  ////////////////////////////////////////////////////////////////
  // every time this function is called, this->msUpdateRate / CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS has elapsed
  // todo: use actual elapsed time?
  nextDutyCycleCalc -= CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS;

  if (nextDutyCycleCalc <= 0)
  {
    // update duty cycle
    float tempError = tempRef - temp;
    float ctrlTempError = kp * tempError;

    // quantize error to DAC output
    int8_t onDurationDacSteps = (int8_t)round(ctrlTempError * CTRL_HEATER_DAC_STEP_SIZE);
    onDurationDacSteps = onDurationDacSteps >= CTRL_HEATER_DAC_MAX_VALUE ? CTRL_HEATER_DAC_MAX_VALUE : onDurationDacSteps;
    onDurationDacSteps = onDurationDacSteps < 0 ? 0 : onDurationDacSteps; // todo: negative temp's are not handled great
    onDurationTime = onDurationDacSteps * CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS;

    // reset next duty cycle update time
    nextDutyCycleCalc = CTRL_HEATER_DUTY_CYCLE_INTERVAL;
  }

  // update actuator aka SSR / heater
  this->activateHeater = onDurationTime < nextDutyCycleCalc ? true : false;
}
