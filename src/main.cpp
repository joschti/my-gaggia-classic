//-----------------------------------------------------------------
/* Includes */
// Arduino
#include <Arduino.h>

// My types
#include "mytypes.h"

// My drivers
#include "alive_led.hpp"
#include "serialout.hpp"
#include "displayout.hpp"
#include "drv_smt172.hpp"
#include "myble.hpp"

//-----------------------------------------------------------------
/* Defines */
// Pins
#define ALIVE_LED_PIN P0_16
#define SMT172_RISE_PIN P0_4
#define SMT172_FALL_PIN P0_5
#define SMT172_TIMER_INST NRFX_TIMER_INSTANCE(1)

// Intervals
#define ALIVE_LED_INTERVAL 500
#define SERIAL_OUTPUT_INTERVAL 500
#define BLE_OUTPUT_INTERVAL 500

//-----------------------------------------------------------------
/* Function prototypes */
static void smtGpioteInterruptHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

//-----------------------------------------------------------------
/* Global variables */
AliveLed aliveLed(ALIVE_LED_PIN, ALIVE_LED_INTERVAL);
SerialOut serialOut(SERIAL_OUTPUT_INTERVAL);
DisplayOut displayOut(500);
DrvSmt172 smt(SMT172_RISE_PIN, SMT172_FALL_PIN, SMT172_TIMER_INST, smtGpioteInterruptHandler);
MyBle myble(BLE_OUTPUT_INTERVAL);

//-----------------------------------------------------------------
/* Arduino function definitions */
void setup()
{
  // init serial interface
  Serial.begin(115200);

  Serial.println("Hello from your Gaggia Classic extension");

  // // print mbed version
  // char buffer[20];
  // sprintf(buffer, "mbed v%d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  // Serial.print(buffer);

  // init OLED display
  displayOut.init();

  // init BLE
  myble.init();
}

void loop()
{
  static interface_ble_t interfaceBle = {0};

  static uint32_t lastMs = 0;
  uint32_t currMs = millis();
  if (currMs < lastMs)
  {
    Serial.println("ovf ms");
  }

  aliveLed.taskHandler(currMs);

  float temp = smt.getTemperature();
  if (temp != interfaceBle.payload.temp)
  {
    interfaceBle.payload.temp = temp;
    interfaceBle.update.temp = true;
  }

  serialOut.taskHandler(currMs, interfaceBle.payload.temp);
  displayOut.taskHandler(currMs, interfaceBle.payload.temp);
  myble.taskHandler(currMs, interfaceBle);

  lastMs = currMs;
}

static void smtGpioteInterruptHandler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  smt.onPinEdge(pin, action);
}
