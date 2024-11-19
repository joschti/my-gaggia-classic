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
#define ALIVE_LED_PIN P1_11
#define HARDFAULT_LED_PIN P1_15
#define COFFEE_EXTRACTION_ACTIVE_PIN P1_13
#define COFFEE_EXTRACTION_LED_PIN P1_12
#define SMT172_RISE_PIN P0_4
#define SMT172_FALL_PIN P0_5
#define SMT172_TIMER_INST NRFX_TIMER_INSTANCE(1)
#define CTRL_HEATER_PIN P0_30

// Intervals
#define ALIVE_LED_INTERVAL 1000
#define COFFEE_EXTRACTION_LED_INTERVAL 250
#ifdef SERIALOUT_ENABLE
#define SERIAL_OUTPUT_INTERVAL 500
#endif
#ifdef DISPLAYOUT_ENABLE
#define DISPLAY_OUTPUT_INTERVAL 1000
#endif
#ifdef BLE_ENABLE
#define BLE_OUTPUT_INTERVAL 2000
#endif

//-----------------------------------------------------------------
/* Function prototypes */
static void smtGpioteInterruptHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

//-----------------------------------------------------------------
/* Global variables */
AliveLed aliveLed(ALIVE_LED_PIN, ALIVE_LED_INTERVAL);
mbed::DigitalOut hardFaultLed(HARDFAULT_LED_PIN, true); // pull-down to GND for indication
mbed::DigitalIn coffeeExtractionActive(COFFEE_EXTRACTION_ACTIVE_PIN, PullDown);
AliveLed coffeeExtractionLed(COFFEE_EXTRACTION_LED_PIN, COFFEE_EXTRACTION_LED_INTERVAL);
ControlHeater ctrlHeater(CTRL_HEATER_PIN, CTRL_HEATER_PID);
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
  if (coffeeExtractionActive)
  {
    // activate control boost mode
    // expect a huge temperature drop because of coffee extraction
    ctrlHeater.enterBoostMode();
    coffeeExtractionLed.taskHandler(currMs);
  }
  else
  {
    ctrlHeater.exitBoostMode();
    // force led off (by setting to true)
    coffeeExtractionLed.on();
  }
#ifdef SERIALOUT_ENABLE
  serialOut.taskHandler(currMs, temp, ctrlHeater.isHeaterActive());
#endif
#ifdef DISPLAYOUT_ENABLE
  displayOut.taskHandler(currMs, temp);
#endif
#ifdef BLE_ENABLE
  interfaceBle.payload.temp = temp;
  myble.taskHandler(currMs, interfaceBle);
#endif
  ctrlHeater.taskHandler(currMs, temp);

  lastMs = currMs;
}

static void smtGpioteInterruptHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  smt.onPinEdge(pin, action);
}

void HardFault_Handler(void)
{
  // hardfault indication
  hardFaultLed = !hardFaultLed;
  ctrlHeater.stopHeater();

  // until we go for the system reset option or watchdog solution
  while (true)
    ;

  // let's try a system reset
  // todo: test this, I want to know if we really experience hardfaults
  // NVIC_SystemReset();
}