#include "drv_smt172.hpp"

// for some reason channel 0 seems to be damaged on TIMER1
// we just go for channel 1 and 2 in every case
#define TIMER_CC_CH1 NRF_TIMER_CC_CHANNEL1
#define TIMER_CC_CH2 NRF_TIMER_CC_CHANNEL2

static void timerDummyInterruptHandler(nrf_timer_event_t event_type, void *p_context)
{
}

static uint32_t sub_u32_overflow_handled(uint32_t val1, uint32_t val2)
{
  // return (val1 - val2)
  uint32_t res;

  if (val1 >= val2)
  {
    // all good
    res = val1 - val2;
  }
  else
  {
    // overflow
    res = val1 + (0xFFFFFFFFu - val2 + 1);
  }
  return res;
}

DrvSmt172::DrvSmt172(nrfx_gpiote_pin_t smtPinRise,
                     nrfx_gpiote_pin_t smtPinFall,
                     nrfx_timer_t smtTimer,
                     nrfx_gpiote_evt_handler_t smtIntHandler)
{
  // error handling return value
  nrfx_err_t ret;

  // populate / initialize member variables
  this->smtPinRise = smtPinRise;
  this->smtPinFall = smtPinFall;
  this->smtTimer = smtTimer;
  this->pwmDutyCycleRawNextElement = 0;
  memset(this->pwmDutyCycleRaw, 0, sizeof(this->pwmDutyCycleRaw));
  this->pwmDutyCycleFilt = 0.0f;
  this->tempFilt = 0.0f;
  this->firstRisingEdgeTimeUs = 0;
  this->secondRisingEdgeTimeUs = 0;
  this->fallingEdgeTimeUs = 0;
  this->periodUs = 0;

  // GPIOTE interrupt handler needs to call the objects internal SMT172 interrupt handler
  this->smtIntHandler = smtIntHandler;

  // init GPIOTE module
  if (!nrfx_gpiote_is_init())
  {
    ret = nrfx_gpiote_init();
    APP_ERROR_CHECK(ret);
  }

  // configure SMT172 timer
  nrfx_timer_config_t smtTimerConfig = {
      .frequency = NRF_TIMER_FREQ_16MHz,
      .mode = NRF_TIMER_MODE_TIMER, // clock is the source
      .bit_width = NRF_TIMER_BIT_WIDTH_32,
      .interrupt_priority = NRFX_TIMER_DEFAULT_CONFIG_IRQ_PRIORITY,
      .p_context = NULL,
  };
  ret = nrfx_timer_init(&this->smtTimer, &smtTimerConfig, timerDummyInterruptHandler);
  APP_ERROR_CHECK(ret);

  // configure SMT172 input pins
  // need two pins because with just one, we cannot differentiate between rising and falling edges
  // configure first one for the rising edge
  nrfx_gpiote_in_config_t smtPin1Config = {
      .sense = NRF_GPIOTE_POLARITY_LOTOHI,
      .pull = NRF_GPIO_PIN_NOPULL,
      .is_watcher = false,
      .hi_accuracy = true,
      .skip_gpio_setup = false,
  };
  ret = nrfx_gpiote_in_init(this->smtPinRise, &smtPin1Config, this->smtIntHandler);
  APP_ERROR_CHECK(ret);

  // configure second one for the falling edge
  nrfx_gpiote_in_config_t smtPin2Config = {
      .sense = NRF_GPIOTE_POLARITY_HITOLO,
      .pull = NRF_GPIO_PIN_NOPULL,
      .is_watcher = false,
      .hi_accuracy = true,
      .skip_gpio_setup = false,
  };
  ret = nrfx_gpiote_in_init(this->smtPinFall, &smtPin2Config, this->smtIntHandler);
  APP_ERROR_CHECK(ret);

  // configure two PPI channels to
  // - event on both edges (rising / falling) of GPIO pins
  // - route to TIMER capture the time on both edges
  ///////////////////////////////////////////////////////////////////////////////
  // configure first PPI channel
  nrf_ppi_channel_t ppi_chn1;
  ret = nrfx_ppi_channel_alloc(&ppi_chn1);
  APP_ERROR_CHECK(ret);

  // get event and task addresses
  uint32_t gpiote_evt_addr1 = nrfx_gpiote_in_event_addr_get(this->smtPinRise);
  uint32_t capture_task_addr1 = nrfx_timer_capture_task_address_get(&this->smtTimer, TIMER_CC_CH1);

  // assign and enable PPI channel
  ret = nrfx_ppi_channel_assign(ppi_chn1, gpiote_evt_addr1, capture_task_addr1);
  APP_ERROR_CHECK(ret);
  ret = nrfx_ppi_channel_enable(ppi_chn1);
  APP_ERROR_CHECK(ret);

  ///////////////////////////////////////////////////////////////////////////////
  // configure second PPI channel
  nrf_ppi_channel_t ppi_chn2;
  ret = nrfx_ppi_channel_alloc(&ppi_chn2);
  APP_ERROR_CHECK(ret);

  // get event and task addresses
  uint32_t gpiote_evt_addr2 = nrfx_gpiote_in_event_addr_get(this->smtPinFall);
  uint32_t capture_task_addr2 = nrfx_timer_capture_task_address_get(&smtTimer, TIMER_CC_CH2);

  // assign and enable PPI channel
  ret = nrfx_ppi_channel_assign(ppi_chn2, gpiote_evt_addr2, capture_task_addr2);
  APP_ERROR_CHECK(ret);
  ret = nrfx_ppi_channel_enable(ppi_chn2);
  APP_ERROR_CHECK(ret);

  // enable peripherals
  nrfx_timer_enable(&this->smtTimer);
  nrfx_gpiote_in_event_enable(this->smtPinRise, true);
  nrfx_gpiote_in_event_enable(this->smtPinFall, true);
}

void DrvSmt172::onPinEdge(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  // note: we guarantee that the interrupt handler is slower than PPI to capture
  // PPI is synchronized with 16 MHz clock, so we need to account for one period of that
  // CPU clock speed is 64 MHz, so 4 clock cycles give us the min. margin we need
  _NOP();
  _NOP();
  _NOP();
  _NOP();

  // need to differentiate between rising and falling edges to calculate the duty cycle
  if (action == NRF_GPIOTE_POLARITY_LOTOHI)
  {
    // rising edge
    // this triggers these things:
    //   - copy the rising time to the first one
    //   - measure the rising time
    //   - calculate the duty cycle
    //   - reset the counter register
    this->firstRisingEdgeTimeUs = this->secondRisingEdgeTimeUs;
    this->secondRisingEdgeTimeUs = nrfx_timer_capture_get(&this->smtTimer, TIMER_CC_CH1);
    this->periodUs = sub_u32_overflow_handled(this->secondRisingEdgeTimeUs, this->firstRisingEdgeTimeUs);
    if (this->periodUs)
    {
      // avoid division by 0
      this->calcPwmDutyCycle();
    }
  }
  else if (action == NRF_GPIOTE_POLARITY_HITOLO)
  {
    // falling edge
    this->fallingEdgeTimeUs = nrfx_timer_capture_get(&this->smtTimer, TIMER_CC_CH2);
  }
  else
  {
    // should never land here
  }
}

void DrvSmt172::calcPwmDutyCycle()
{
  // calculate the duty cycle of the PWM
  uint32_t onPeriodUs = sub_u32_overflow_handled(this->fallingEdgeTimeUs, this->firstRisingEdgeTimeUs);
  this->pwmDutyCycleRaw[this->pwmDutyCycleRawNextElement] = ((float)(onPeriodUs)) / (float)this->periodUs;
  this->pwmDutyCycleRawNextElement =
      this->pwmDutyCycleRawNextElement < (DRV_SMT172_MA_LEN - 1) ? this->pwmDutyCycleRawNextElement + 1 : 0;

  // average the duty cycle as instructed by data sheet
  this->averagePwmDutyCycle();
}

void DrvSmt172::averagePwmDutyCycle()
{
  // reset filter
  this->pwmDutyCycleFilt = 0.0f;

  // simple and inefficient MA implementation
  for (uint8_t i = 0; i < DRV_SMT172_MA_LEN; i++)
  {
    this->pwmDutyCycleFilt += this->pwmDutyCycleRaw[i];
  }
  this->pwmDutyCycleFilt /= DRV_SMT172_MA_LEN;
}

float DrvSmt172::getTemperature()
{
#if DRV_SMT172_HIGH_PRECISION_CALC
  this->tempFilt = -1.43f * (this->pwmDutyCycleFilt * this->pwmDutyCycleFilt) + 214.56f * this->pwmDutyCycleFilt - 68.60f;
#else
  this->tempFilt = 212.77f * this->pwmDutyCycleFilt - 68.085f;
#endif
  return this->tempFilt;
}