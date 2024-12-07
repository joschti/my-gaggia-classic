#include "ctrl_heater.hpp"
#include "config.hpp"

#define CTRL_HEATER_MIN_VALID_TEMP 0   //! temperature will never go below during normal operation
#define CTRL_HEATER_MAX_VALID_TEMP 150 //! temperature will never go higher during normal operation

#define CTRL_HEATER_DAC_RES 5 //! equivalent to 5 bits DAC resolution
#define CTRL_HEATER_DAC_STEP_SIZE 1
#define CTRL_HEATER_DAC_NUM_STEPS (1 << CTRL_HEATER_DAC_RES)
#define CTRL_HEATER_DAC_MAX_VALUE (CTRL_HEATER_DAC_NUM_STEPS - 1)
#define CTRL_HEATER_DUTY_CYCLE_INTERVAL (CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS * CTRL_HEATER_DAC_NUM_STEPS) //! (20 * 32) = 640 ms
#define CTRL_HEATER_PID_MAX_INT_DUR 3000                                                                 //! [s], over which (max.) time interval the I part is integrating over
#define CTRL_HEATER_PID_MEMORY (CTRL_HEATER_PID_MAX_INT_DUR / CTRL_HEATER_DUTY_CYCLE_INTERVAL)           //! array size for past temperature errors

#if (CTRL_HEATER_PID_MEMORY < 2)
#error PID: with these settings, I part would not integrate!
#endif

#define CTRL_HEATER_BOOST_GAIN 10.0
#define CTRL_HEATER_BOOST_NEGATIVE_TEMP_DUTY_CYCLE 0.5

ControlHeater::ControlHeater(PinName activateHeaterPin, ctrl_heater_t type)
    : MyTask(CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS), // ensure we use max SSR frequency
      activateHeater(activateHeaterPin),
      type(type)
{
}

void ControlHeater::taskHandler(float temp)
{
  if (this->timeHandler())
  {
    if (!isTemperatureInValidRange(temp))
    {
      // temperature outside of value range
      // -> immediately turn heater off
      this->activateHeater = false;
      return;
    }

    switch (this->type)
    {
    case CTRL_HEATER_TWO_POINT:
      ctrl_two_point(temp);
      break;
    case CTRL_HEATER_PID:
      ctrl_pid(temp);
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

bool ControlHeater::isTemperatureInValidRange(float temp)
{
  if (CTRL_HEATER_MAX_VALID_TEMP > temp && temp > CTRL_HEATER_MIN_VALID_TEMP)
  {
    return true;
  }
  else
  {
    return false;
  }
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
void ControlHeater::ctrl_pid(float temp)
{
  ////////////////////////////////////////////////////////////////
  // controller parameterization
  ////////////////////////////////////////////////////////////////
  const float tempRef = config::targetTemp;
  const float kp = !this->isBoostActive ? config::ctrlHeaterProp : CTRL_HEATER_BOOST_GAIN * config::ctrlHeaterProp;
  const float ki = config::ctrlHeaterInt;
  const float kd = config::ctrlHeaterDiff;

  ////////////////////////////////////////////////////////////////
  // static memory
  ////////////////////////////////////////////////////////////////
  static uint16_t onDurationTime = 0;
  // every CTRL_HEATER_DUTY_CYCLE_INTERVAL, actual duty cycle is recalculated and updated
  static int16_t nextDutyCycleCalc = 0;
  // store past seen temperature errors for integration and derivative parts (entry 0 is the latest one)
  static float tempErrorMemory[CTRL_HEATER_PID_MEMORY] = {0};

  ////////////////////////////////////////////////////////////////
  // controller update
  ////////////////////////////////////////////////////////////////
  // every time this function is called, this->msUpdateRate / CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS has elapsed
  // todo: use actual elapsed time?
  nextDutyCycleCalc -= CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS;

  if (nextDutyCycleCalc <= 0)
  {
    float tempError = tempRef - temp;

    // store temperature error in static memory but first we need to move the entries by one
    // and delete the oldest value
    for (uint8_t i = 0; i < CTRL_HEATER_PID_MEMORY - 1; i++)
    {
      tempErrorMemory[i + 1] = tempErrorMemory[i];
    }
    tempErrorMemory[0] = tempError;

    // calculate the integration part
    float tempErrorInt = 0;
    for (uint8_t i = 0; i < CTRL_HEATER_PID_MEMORY; i++)
    {
      tempErrorInt += tempErrorMemory[i];
    }

    // calculate the derivative part (previous - latest)
    // todo: is the sign correct? I think not!
    // todo: learn from https://www.zhinst.com/sites/default/files/documents/2023-08/zi_whitepaper_principles_of_pid_controllers.pdf
    // "purpose of the derivative action is to increase the damping of the system"
    // examples:
    // - [0]=-10, [1]=-5 -> error increases -> diff=+5 (need temp. increase)
    // - [0]=+10, [1]=+5 -> error increases -> diff=-5 (need temp. decrease)
    // todo: low-pass filter use simple IIR exponential smoothing filter
    float tempErrorDiff = tempErrorMemory[1] - tempErrorMemory[0];

    // combine P, I and D parts and update intermediate control temperature error (basis for duty cycle)
    float ctrlTempError = kp * tempError;
    ctrlTempError += ki * tempErrorInt;  // todo: ki is unspecific, it seems to be some sort of mixture
    ctrlTempError += kd * tempErrorDiff; // todo: kd is unspecific, should it have seconds as unit?

    // quantize error to DAC output
    int16_t onDurationDacSteps = (int16_t)round(ctrlTempError * CTRL_HEATER_DAC_STEP_SIZE);
    onDurationDacSteps = onDurationDacSteps >= CTRL_HEATER_DAC_MAX_VALUE ? CTRL_HEATER_DAC_MAX_VALUE : onDurationDacSteps;
    onDurationDacSteps = onDurationDacSteps < 0 ? 0 : onDurationDacSteps; // todo: negative temp's are not handled great
    onDurationTime = onDurationDacSteps * CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS;

    // initial boost mode behaviour
    if (this->isBoostActive)
    {
      // in case of negative temperature error:
      // boost mode should bring the heater to an initial temperature to improve its responsiveness
      // in case of positive temperature error:
      // boost mode uses a more aggressive PID configuration
      onDurationTime = onDurationTime == 0 ? CTRL_HEATER_BOOST_NEGATIVE_TEMP_DUTY_CYCLE * CTRL_HEATER_DUTY_CYCLE_INTERVAL : onDurationTime;
    }

    // anti-windup
    if (onDurationDacSteps == CTRL_HEATER_DAC_MAX_VALUE)
    {
      // if we hit the limit of our DAC, we reset the integral part
      tempErrorInt = 0;
    }

    // reset next duty cycle update time
    nextDutyCycleCalc = CTRL_HEATER_DUTY_CYCLE_INTERVAL;
  }

  // update actuator aka SSR / heater
  this->activateHeater = onDurationTime < nextDutyCycleCalc ? false : true;
}

void ControlHeater::stopHeater(void)
{
  // only called in hardfault handler, so setting it to false is enough
  // no need to block it from returning to true
  this->activateHeater = false;
}

void ControlHeater::enterBoostMode(void)
{
  this->isBoostActive = true;
}

void ControlHeater::exitBoostMode(void)
{
  this->isBoostActive = false;
}
