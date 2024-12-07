#ifndef _DISPLAYOUT_HPP_
#define _DISPLAYOUT_HPP_

#ifdef DISPLAYOUT_ENABLE

#include "Arduino.h"
#include "mytask.hpp"

#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_ILI9341.h>
#include "drv_ili9341.h"

//-----------------------------------------------------------------
/* Defines */
// Pins
// #define DISPLAY_IO_MOSI P1_1
// #define DISPLAY_IO_MISO P1_8
// #define DISPLAY_IO_SCLK P0_13
#define DISPLAY_IO_DC P0_27
#define DISPLAY_IO_CS P1_2
#define DISPLAY_IO_RST P0_21

// Others
// #define DISPLAY_IO_SPI_FREQ 10e6
#define DISPLAY_COLOR_BW DRV_ILI9341_BLACK
#define DISPLAY_COLOR_TEXT DRV_ILI9341_WHITE

//-----------------------------------------------------------------
/* Classes */
class DisplayOut : public MyTask
{
public:
  DisplayOut(uint32_t msUpdateRate) : MyTask(msUpdateRate),
                                      display(this->pixels, DISPLAY_IO_DC, DISPLAY_IO_RST, DISPLAY_IO_CS)
  {
  }

  void init()
  {
    // init
    this->display.begin();

    // set the background in memory
    memset(this->pixels, DISPLAY_COLOR_BW, DRV_ILI9341_TFTHEIGHT * DRV_ILI9341_TFTWIDTH);

#ifndef DRV_ILI9341_RAM_FOR_SPEED
    // black start screen
    this->display.fillScreen(DISPLAY_COLOR_BW);
#endif

    // settings
    this->display.setTextSize(3);
    this->display.setTextColor(DISPLAY_COLOR_TEXT);
    this->display.setRotation(1);

    // static display part
    this->display.setCursor(0, 0);
    this->display.println("My Gaggia Classic");
    this->display.setCursor(0, 50);
    this->display.println("Temperature:");

    // bring memory image to display
    this->display.drawBuffer(this->pixels);
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
      const int16_t tempTextPosX = 0;
      const int16_t tempTextPosY = 75;

      // remove temperature value from display
      this->display.setCursor(tempTextPosX, tempTextPosY);
      // int16_t dummyX, dummyY;
      // uint16_t tempTextWidth, tempTextHeight;
      // this->display.getTextBounds(this->tempStrBuffer, tempTextPosX, tempTextPosY, &dummyX, &dummyY, &tempTextWidth, &tempTextHeight);
      // this->display.fillRect(tempTextPosX, tempTextPosY, tempTextWidth, tempTextHeight, DISPLAY_COLOR_BW);
      this->display.setTextColor(DISPLAY_COLOR_BW);
      this->display.print(this->tempStrBuffer);

      // write temperature value onto display
      this->display.setCursor(tempTextPosX, tempTextPosY);
      this->display.setTextColor(DISPLAY_COLOR_TEXT);
      sprintf(this->tempStrBuffer, "%4.2f %cC", temp, 0xF7);
      this->display.print(this->tempStrBuffer);

      // bring memory image to display
      this->display.drawBuffer(this->pixels);
    }
  }

private:
  DrvIli9341 display;
  char tempStrBuffer[20];
  uint16_t pixels[DRV_ILI9341_TFTHEIGHT * DRV_ILI9341_TFTWIDTH];
};

#endif

#endif
