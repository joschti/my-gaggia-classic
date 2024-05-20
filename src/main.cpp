//-----------------------------------------------------------------
/* Includes */
// Arduino
#include <Arduino.h>

// My types
#include "mytypes.h"

// My drivers
#include "alive_led.hpp"
#include "drv_smt172.hpp"
#include "ctrl_heater.hpp"
#ifdef SERIALOUT_ENABLE
#include "serialout.hpp"
#endif
#ifdef DISPLAYOUT_ENABLE
#include "displayout.hpp"
#endif
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
#ifdef DISPLAYOUT_ENABLE
#define DISPLAY_OUTPUT_INTERVAL 1000
#endif
#ifdef BLE_ENABLE
#define BLE_OUTPUT_INTERVAL 500
#endif

//-----------------------------------------------------------------
/* Function prototypes */
static void smtGpioteInterruptHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

//-----------------------------------------------------------------
/* Global variables */
AliveLed aliveLed(ALIVE_LED_PIN, ALIVE_LED_INTERVAL);
ControlHeater ctrlHeater(CTRL_HEATER_PIN, CTRL_HEATER_TWO_POINT);
#ifdef SERIALOUT_ENABLE
SerialOut serialOut(SERIAL_OUTPUT_INTERVAL);
#endif
#ifdef DISPLAYOUT_ENABLE
DisplayOut displayOut(DISPLAY_OUTPUT_INTERVAL);
#endif
DrvSmt172 smt(SMT172_RISE_PIN, SMT172_FALL_PIN, SMT172_TIMER_INST, smtGpioteInterruptHandler);
#ifdef BLE_ENABLE
MyBle myble(BLE_OUTPUT_INTERVAL);
#endif

//-----------------------------------------------------------------
/* Arduino function definitions */
void setup()
{
  // init
#ifdef SERIALOUT_ENABLE
  Serial.begin(115200);
#endif
#ifdef DISPLAYOUT_ENABLE
  displayOut.init();
#endif
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
#ifdef DISPLAYOUT_ENABLE
  displayOut.taskHandler(currMs, interfaceBle.payload.temp);
#endif
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
