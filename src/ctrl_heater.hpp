#ifndef _CONTROL_HEATER_HPP_
#define _CONTROL_HEATER_HPP_

#include "Arduino.h"
#include "mbed.h"
#include "mytask.hpp"

typedef enum
{
  CTRL_HEATER_T_TWO_POINT,
  CTRL_HEATER_T_P,
  CTRL_HEATER_T_PID,
  CTRL_HEATER_T_KALMAN
} ctrl_heater_t;

class ControlHeater : public MyTask
{
public:
  ControlHeater(PinName activateHeaterPin, ctrl_heater_t type, uint32_t msUpdateRate) : MyTask(msUpdateRate), activateHeater(activateHeaterPin), type(type) {}

  void taskHandler(uint32_t currMs, float temp)
  {
    if (this->timeHandler(currMs))
    {
      switch (this->type)
      {
      case CTRL_HEATER_T_TWO_POINT:
        ctrl_two_point(temp);
        break;
      case CTRL_HEATER_T_P:
        ctrl_proportional(temp);
        break;
      case CTRL_HEATER_T_PID:
        // not implemented yet
        break;
      case CTRL_HEATER_T_KALMAN:
        // not implemented yet
        break;
      }
    }
  }

  bool isHeaterActive()
  {
    return this->activateHeater ? true : false;
  }

private:
  mbed::DigitalOut activateHeater;
  ctrl_heater_t type;

  //! 2-point temperature controller implementation
  void ctrl_two_point(float temp)
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
  void ctrl_proportional(float temp)
  {
    ////////////////////////////////////////////////////////////////
    // controller parameterization
    ////////////////////////////////////////////////////////////////
    const float tempRef = 40.0f;
    const uint16_t dutyCycleTime = 1000; // ms
    const float kp = 1.0f;
    // todo: instead of this weird transformation, just use quantization
    //       and we treat duty cycle like number of quantization bits.
    const float tempError2DutyCycle = 1.0f;

    ////////////////////////////////////////////////////////////////
    // static memory
    ////////////////////////////////////////////////////////////////
    static float dutyCycle = 0;
    static uint16_t onDuration = 0;
    // every dutyCycleTime, actual duty cycle is recalculated and updated
    static int16_t nextDutyCycleCalc = dutyCycleTime;

    ////////////////////////////////////////////////////////////////
    // controller update
    ////////////////////////////////////////////////////////////////
    // every time this function is called, this->msUpdateRate has elapsed
    // todo: use actual elapsed time
    nextDutyCycleCalc -= (int16_t)this->getMsUpdateRate();

    if (nextDutyCycleCalc <= 0)
    {
      // update duty cycle
      float tempError = tempRef - temp;
      float ctrlTempError = kp * tempError;

      // need to transform the controller error to a duty cycle
      // min / max: 0 and 1
      dutyCycle = tempError2DutyCycle * ctrlTempError;
      dutyCycle = dutyCycle >= 1.0f ? 1.0f : dutyCycle;
      dutyCycle = dutyCycle < 0.0f ? 0.0f : dutyCycle; // todo: negative temp's are not handled great
      onDuration = (uint16_t)(dutyCycle * dutyCycleTime);

      // reset next duty cycle update time
      nextDutyCycleCalc = dutyCycleTime;
    }

    // update actuator aka SSR / heater
    this->activateHeater = onDuration < nextDutyCycleCalc ? true : false;
  }
};

#endif
