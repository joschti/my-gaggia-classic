#ifndef _DISPLAYOUT_HPP_
#define _DISPLAYOUT_HPP_

#include "Arduino.h"
#include "mytask.hpp"

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);

class DisplayOut : public MyTask
{
public:
  DisplayOut(uint32_t msUpdateRate) : MyTask(msUpdateRate),
                                      display(64, 128, &Wire)
  {
  }

  void init()
  {
    delay(250);                      // wait for the OLED to power up
    this->display.begin(0x3C, true); // Address 0x3C default

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    this->display.display();
    delay(1000);

    // Clear the buffer.
    this->display.clearDisplay();
    this->display.display();

    this->display.setRotation(1);
    // Serial.println("Button test");

    // text display tests
    this->display.setTextSize(2);
    this->display.setTextColor(SH110X_WHITE);
    this->display.setCursor(0, 0);
  }

  void taskHandler(uint32_t currMs, float temp)
  {
    if (this->timeHandler(currMs))
    {
      this->display.clearDisplay();
      this->display.setCursor(0, 0);
      char buffer[30];
      sprintf(buffer, "Temp\n%4.2f %cC", temp, 0xF7);
      this->display.print(buffer);
      delay(10);
      yield();
      this->display.display();
    }
  }

private:
  Adafruit_SH1107 display;
};

#endif
