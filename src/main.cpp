//-----------------------------------------------------------------
/* Includes */
// Arduino
#include <Arduino.h>

// My types
#include "mytypes.h"

// My drivers
#include "alive_led.hpp"
#ifdef SERIALOUT_ENABLE
#include "serialout.hpp"
#endif
#include "displayout.hpp"
#include "drv_smt172.hpp"
#include "ctrl_heater.hpp"
#ifdef BLE_ENABLE
#include "myble.hpp"
#endif

//-----------------------------------------------------------------
/* Defines */
// Pins
#define ALIVE_LED_PIN P0_16
#define SMT172_RISE_PIN P0_4
#define SMT172_FALL_PIN P0_5
#define SMT172_TIMER_INST NRFX_TIMER_INSTANCE(1)
#define CTRL_HEATER_PIN P0_30

// Intervals
#define ALIVE_LED_INTERVAL 1000
#ifdef SERIALOUT_ENABLE
#define SERIAL_OUTPUT_INTERVAL 500
#endif
#define DISPLAY_OUTPUT_INTERVAL 1000
#ifdef BLE_ENABLE
#define BLE_OUTPUT_INTERVAL 500
#endif
#define CTRL_HEATER_INTERVAL 100 // cannot be smaller than 20 ms because SSR operates on this rate

//-----------------------------------------------------------------
/* Function prototypes */
static void smtGpioteInterruptHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

//-----------------------------------------------------------------
/* Global variables */
AliveLed aliveLed(ALIVE_LED_PIN, ALIVE_LED_INTERVAL);
#ifdef SERIALOUT_ENABLE
SerialOut serialOut(SERIAL_OUTPUT_INTERVAL);
#endif
DisplayOut displayOut(DISPLAY_OUTPUT_INTERVAL);
DrvSmt172 smt(SMT172_RISE_PIN, SMT172_FALL_PIN, SMT172_TIMER_INST, smtGpioteInterruptHandler);
#ifdef BLE_ENABLE
MyBle myble(BLE_OUTPUT_INTERVAL);
#endif
ControlHeater ctrlHeater(CTRL_HEATER_PIN, CTRL_HEATER_T_TWO_POINT, CTRL_HEATER_INTERVAL);

//-----------------------------------------------------------------
/* Arduino function definitions */
void setup()
{
  // init
#ifdef SERIALOUT_ENABLE
  Serial.begin(115200);
#endif
  displayOut.init();
#ifdef BLE_ENABLE
  myble.init();
#endif
}

void loop()
{
  static interface_ble_t interfaceBle = {0};

  static uint32_t lastMs = 0;
  uint32_t currMs = millis();
  if (currMs < lastMs)
  {
#ifdef SERIALOUT_ENABLE
    Serial.println("ovf ms");
#endif
  }

  float temp = smt.getTemperature();

  aliveLed.taskHandler(currMs);
#ifdef SERIALOUT_ENABLE
  serialOut.taskHandler(currMs, interfaceBle.payload.temp, ctrlHeater.isHeaterActive());
#endif
  displayOut.taskHandler(currMs, interfaceBle.payload.temp);
#ifdef BLE_ENABLE
  if (temp != interfaceBle.payload.temp)
  {
    interfaceBle.payload.temp = temp;
    interfaceBle.update.temp = true;
  }
  myble.taskHandler(currMs, interfaceBle);
#endif
  ctrlHeater.taskHandler(currMs, temp);

  lastMs = currMs;
}

static void smtGpioteInterruptHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  smt.onPinEdge(pin, action);
}
