#ifndef _DRV_SMT172_HPP_
#define _DRV_SMT172_HPP_

#include "Arduino.h"

// NRF drivers
#include "nrfx.h"
#include "nrfx_gpiote.h"
#include "nrfx_timer.h"
#include "nrfx_ppi.h"

#define DRV_SMT172_MA_LEN 8
#define DRV_SMT172_HIGH_PRECISION_CALC true

class DrvSmt172
{
public:
  DrvSmt172(nrfx_gpiote_pin_t smtPinRise,
            nrfx_gpiote_pin_t smtPinFall,
            nrfx_timer_t smtTimer,
            nrfx_gpiote_evt_handler_t smtIntHandler);
  void onPinEdge(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);
  float getTemperature();

private:
  nrfx_gpiote_pin_t smtPinRise;
  nrfx_gpiote_pin_t smtPinFall;
  nrfx_timer_t smtTimer;
  uint8_t pwmDutyCycleRawNextElement;
  float pwmDutyCycleRaw[DRV_SMT172_MA_LEN];
  float pwmDutyCycleFilt;
  float tempFilt;
  uint32_t firstRisingEdgeTimeUs;
  uint32_t secondRisingEdgeTimeUs;
  uint32_t fallingEdgeTimeUs;
  uint32_t periodUs;
  nrfx_gpiote_evt_handler_t smtIntHandler;

  void calcPwmDutyCycle();
  void averagePwmDutyCycle();
};

#endif
