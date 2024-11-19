#ifndef _CONTROL_HEATER_HPP_
#define _CONTROL_HEATER_HPP_

#include "Arduino.h"
#include "mbed.h"
#include "mytask.hpp"

#define CTRL_HEATER_MIN_SSR_SWITCH_TIME_MS 20 //! min. on/off switch time, we cannot go smaller

typedef enum
{
  CTRL_HEATER_TWO_POINT,
  CTRL_HEATER_PID,
  CTRL_HEATER_KALMAN
} ctrl_heater_t;

class ControlHeater : public MyTask
{
public:
  ControlHeater(PinName activateHeaterPin, ctrl_heater_t type);
  void taskHandler(uint32_t currMs, float temp);
  bool isHeaterActive();
  void stopHeater(void);

  // use boost mode whenever we know that the temperature will drop a lot
  void enterBoostMode(void);
  void exitBoostMode(void);

private:
  mbed::DigitalOut activateHeater;
  ctrl_heater_t type;
  bool isBoostActive;

  bool isTemperatureInValidRange(float temp);

  //! two-point temperature controller implementation
  void ctrl_two_point(float temp);

  //! P temperature controller implementation
  void ctrl_pid(float temp);
};

#endif
