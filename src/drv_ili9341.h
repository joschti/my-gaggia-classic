// Inspired by https://github.com/cbm80amiga/ILI9341_Fast
#ifndef _DRV_ILI9341_H_
#define _DRV_ILI9341_H_

// trade speed for memory
#define DRV_ILI9341_RAM_FOR_SPEED

// define when CS pin is always connected to the ground
#define DRV_ILI9341_CS_ALWAYS_LOW
// ------------------------------

#include "Arduino.h"
#include "Print.h"
#include <Adafruit_GFX.h>
#include <avr/pgmspace.h>
#include <mbed.h>

#define DRV_ILI9341_TFTWIDTH 240
#define DRV_ILI9341_TFTHEIGHT 320

// Color definitions
#define DRV_ILI9341_BLACK 0x0000
#define DRV_ILI9341_BLUE 0x001F
#define DRV_ILI9341_RED 0xF800
#define DRV_ILI9341_GREEN 0x07E0
#define DRV_ILI9341_CYAN 0x07FF
#define DRV_ILI9341_MAGENTA 0xF81F
#define DRV_ILI9341_YELLOW 0xFFE0
#define DRV_ILI9341_WHITE 0xFFFF

#define DRV_ILI9341_RGB_TO_565(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))
#define DRV_ILI9341_GREY DRV_ILI9341_RGB_TO_565(128, 128, 128)
#define DRV_ILI9341_LGREY DRV_ILI9341_RGB_TO_565(160, 160, 160)
#define DRV_ILI9341_DGREY DRV_ILI9341_RGB_TO_565(80, 80, 80)
#define DRV_ILI9341_LBLUE DRV_ILI9341_RGB_TO_565(100, 100, 255)
#define DRV_ILI9341_DBLUE DRV_ILI9341_RGB_TO_565(0, 0, 128)

class DrvIli9341 : public Adafruit_GFX
{

public:
  DrvIli9341(uint16_t *pixels, PinName dc, PinName rst, PinName cs, PinName bl);

  void init();
  void begin() { init(); }
  void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void pushColor(uint16_t color);
  void clearScreen() { fillScreen(DRV_ILI9341_BLACK); }
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void drawBuffer(uint16_t *buffer);
  void setRotation(uint8_t r);

protected:
  void displayInit(const uint8_t *addr);
  void writeSPI(uint8_t);
  void writeMulti(uint16_t color, uint16_t num);
  void writeCmd(uint8_t c);
  void writeData(uint8_t d8);
  void writeData16(uint16_t d16);
  void copyMulti(uint8_t *img, uint16_t num);

private:
  int8_t csPin, dcPin, rstPin;
  uint8_t csMask, dcMask;
  volatile uint8_t *csPort, *dcPort;
  uint16_t *pixels;
  mbed::DigitalOut csGpio, dcGpio, rstGpio, blGpio;

  void handleRotation(int16_t *x, int16_t *y);
};

#endif
