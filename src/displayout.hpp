#ifndef _DISPLAYOUT_HPP_
#define _DISPLAYOUT_HPP_

#include "Arduino.h"
#include "mytask.hpp"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

//-----------------------------------------------------------------
/* Defines */
// Pins
// #define DISPLAY_IO_MOSI P1_1
// #define DISPLAY_IO_MISO P1_8
// #define DISPLAY_IO_SCLK P0_13
// #define DISPLAY_IO_DC P0_27
// #define DISPLAY_IO_CS P1_2
// #define DISPLAY_IO_RST P0_21
#define DISPLAY_IO_DC 9 // library cannot deal with PinName objects
#define DISPLAY_IO_CS 10

// Others
#define DISPLAY_COLOR_BW ILI9341_BLACK
#define DISPLAY_COLOR_TEXT ILI9341_WHITE

//-----------------------------------------------------------------
/* Classes */
class DisplayOut : public MyTask
{
public:
  DisplayOut(uint32_t msUpdateRate) : MyTask(msUpdateRate),
                                      display(DISPLAY_IO_CS, DISPLAY_IO_DC)
  {
  }

  void init()
  {
    // init
    this->display.begin();

    // black start screen
    this->display.fillScreen(DISPLAY_COLOR_BW);

    // settings
    this->display.setTextSize(3);
    this->display.setTextColor(DISPLAY_COLOR_TEXT);
    this->display.setRotation(1);

    // static display
    this->display.setCursor(0, 0);
    this->display.println("My Gaggia Classic");
    this->display.setCursor(0, 50);
    this->display.println("Temperature:");
  }

  void taskHandler(uint32_t currMs, float temp)
  {
    if (this->timeHandler(currMs))
    {
      // we only handle variable objects for execution speed.
      // display is slow.

      ////////////////////////////////////////////////////////////////
      // display temperature
      ////////////////////////////////////////////////////////////////
      // remove temperature value from display
      this->display.setCursor(0, 75);
      this->display.setTextColor(DISPLAY_COLOR_BW);
      this->display.print(this->tempStrBuffer);

      // write temperature value onto display
      this->display.setCursor(0, 75);
      this->display.setTextColor(DISPLAY_COLOR_TEXT);
      sprintf(this->tempStrBuffer, "%4.2f %cC", temp, 0xF7);
      this->display.print(this->tempStrBuffer);
    }
  }

private:
  Adafruit_ILI9341 display;
  char tempStrBuffer[20];
};

#endif
