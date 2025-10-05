// Basis from https://github.com/cbm80amiga/ILI9341_Fast
// but we provide the option trade RAM for speed

#include "drv_ili9341.h"
#include "SPI.h"

// -----------------------------------------
// ILI9341 commands
#define ILI9341_NOP 0x00
#define ILI9341_SWRESET 0x01
#define ILI9341_RDDID 0x04
#define ILI9341_RDDST 0x09
#define ILI9341_RDMODE 0x0A
#define ILI9341_RDMADCTL 0x0B
#define ILI9341_RDPIXFMT 0x0C
#define ILI9341_RDIMGFMT 0x0A
#define ILI9341_RDSELFDIAG 0x0F

#define ILI9341_SLPIN 0x10
#define ILI9341_SLPOUT 0x11
#define ILI9341_PTLON 0x12
#define ILI9341_NORON 0x13

#define ILI9341_INVOFF 0x20
#define ILI9341_INVON 0x21
#define ILI9341_GAMMASET 0x26
#define ILI9341_DISPOFF 0x28
#define ILI9341_DISPON 0x29
#define ILI9341_CASET 0x2A
#define ILI9341_PASET 0x2B
#define ILI9341_RAMWR 0x2C
#define ILI9341_RAMRD 0x2E

#define ILI9341_PTLAR 0x30
#define ILI9341_VSCRDEF 0x33
#define ILI9341_MADCTL 0x36
#define ILI9341_VSCRSADD 0x37
#define ILI9341_PIXFMT 0x3A

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR 0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1 0xC0
#define ILI9341_PWCTR2 0xC1
#define ILI9341_PWCTR3 0xC2
#define ILI9341_PWCTR4 0xC3
#define ILI9341_PWCTR5 0xC4
#define ILI9341_VMCTR1 0xC5
#define ILI9341_VMCTR2 0xC7
#define ILI9341_PWCTRA 0xCB
#define ILI9341_CMD_CF 0xCF

#define ILI9341_RDID1 0xDA
#define ILI9341_RDID2 0xDB
#define ILI9341_RDID3 0xDC
#define ILI9341_RDID4 0xDD

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
#define ILI9341_TIMCTRA 0xE8
#define ILI9341_TIMCTRB 0xE9
#define ILI9341_TIMCTRC 0xEA
#define ILI9341_POWSEQ 0xED
#define ILI9341_CMD_EF 0xEF

#define ILI9341_EN3GAM 0xF2
#define ILI9341_PUMPRAT 0xF7

#define CMD_DELAY 0x80

#define ILI9341_MADCTL_MY 0x80
#define ILI9341_MADCTL_MX 0x40
#define ILI9341_MADCTL_MV 0x20
#define ILI9341_MADCTL_ML 0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH 0x04

static const uint8_t ILI9341_commands[] PROGMEM = {
    22,                            // number of commands
    ILI9341_SWRESET, CMD_DELAY, 5, // 1
    ILI9341_CMD_EF, 3,             // 2
    0x03, 0x80, 0x02,
    ILI9341_CMD_CF, 3, // 3
    0x00, 0xC1, 0x30,
    ILI9341_POWSEQ, 4, // 4
    0x64, 0x03, 0x12, 0x81,
    ILI9341_TIMCTRA, 3, // 5
    0x85, 0x00, 0x78,
    ILI9341_PWCTRA, 5, // 6
    0x39, 0x2C, 0x00, 0x34, 0x02,
    ILI9341_PUMPRAT, 1, // 7
    0x20,
    ILI9341_TIMCTRC, 2, // 8
    0x00, 0x00,
    ILI9341_PWCTR1, 1, // 9 power control
    0x23,              // VRH[5:0]
    ILI9341_PWCTR2, 1, // 10 power control
    0x10,              // SAP[2:0];BT[3:0]
    ILI9341_VMCTR1, 2, // 11 VCM control
    0x3e, 0x28,
    ILI9341_VMCTR2, 1, // 12 VCM control2
    0x86,              // --
    ILI9341_MADCTL, 1, // 13
    (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR),
    ILI9341_PIXFMT, 1, // 14
    0x55,
    ILI9341_FRMCTR1, 2, // 15
    0x00, 0x18,
    ILI9341_DFUNCTR, 3, // 16
    0x08, 0x82, 0x27,
    ILI9341_EN3GAM, 1, // 17 3Gamma Function Disable
    0x00,
    ILI9341_GAMMASET, 1, // 18 Gamma curve selected
    0x01,
    ILI9341_GMCTRP1, 15, // 19 Set Gamma
    0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
    ILI9341_GMCTRN1, 15, // 20
    0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
    ILI9341_SLPOUT, CMD_DELAY, 120, // 21
    ILI9341_DISPON, 0,              // 22
};
// -----------------------------------------
static SPISettings spiSettings;
#define SPI_START SPI.beginTransaction(spiSettings)
#define SPI_END SPI.endTransaction()

// macros for fast DC and CS state changes
#define DC_DATA this->dcGpio = 1;
#define DC_COMMAND this->dcGpio = 0;
#ifndef DRV_ILI9341_CS_ALWAYS_LOW
#define CS_IDLE this->csGpio = 1;
#define CS_ACTIVE this->csGpio = 0;
#else
#define CS_IDLE
#define CS_ACTIVE
#endif

// ----------------------------------------------------------
inline void DrvIli9341::writeSPI(uint8_t c)
{
  SPI.transfer(c);
}

// ----------------------------------------------------------
// fast method to send multiple 16-bit values via SPI
inline void DrvIli9341::writeMulti(uint16_t color, uint16_t num)
{
  while (num--)
  {
    SPI.transfer(color >> 8);
    SPI.transfer(color);
  }
}
// ----------------------------------------------------------
// fast method to send multiple 16-bit values from RAM via SPI
inline void DrvIli9341::copyMulti(uint8_t *img, uint16_t num)
{
  while (num--)
  {
    SPI.transfer(*(img + 1));
    SPI.transfer(*(img + 0));
    img += 2;
  }
}
// ----------------------------------------------------------
DrvIli9341::DrvIli9341(uint16_t *pixels, PinName dc, PinName rst, PinName cs, PinName bl) : Adafruit_GFX(DRV_ILI9341_TFTWIDTH, DRV_ILI9341_TFTHEIGHT),
                                                                                            csGpio(cs, false), dcGpio(dc, false), rstGpio(rst, false), blGpio(bl, true)
{
  this->pixels = pixels;
}

// ----------------------------------------------------------
void DrvIli9341::init()
{
  // initialize SPI
  SPI.begin();
  spiSettings = SPISettings(16000000, MSBFIRST, SPI_MODE0);

  // start device
  this->rstGpio = false;
  delay(50);
  this->rstGpio = true;
  delay(150);

  // initalize device
  this->_width = DRV_ILI9341_TFTWIDTH;
  this->_height = DRV_ILI9341_TFTHEIGHT;
  displayInit(ILI9341_commands);
}

// ----------------------------------------------------------
void DrvIli9341::writeCmd(uint8_t c)
{
  DC_COMMAND;
  CS_ACTIVE;
  SPI_START;

  writeSPI(c);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void DrvIli9341::writeData(uint8_t d8)
{
  DC_DATA;
  CS_ACTIVE;
  SPI_START;

  writeSPI(d8);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void DrvIli9341::writeData16(uint16_t d16)
{
  DC_DATA;
  CS_ACTIVE;
  SPI_START;

  writeMulti(d16, 1);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
void DrvIli9341::displayInit(const uint8_t *addr)
{
  uint8_t numCommands, numArgs;
  uint16_t ms;
  numCommands = pgm_read_byte(addr++);
  while (numCommands--)
  {
    writeCmd(pgm_read_byte(addr++));
    numArgs = pgm_read_byte(addr++);
    ms = numArgs & CMD_DELAY;
    numArgs &= ~CMD_DELAY;
    while (numArgs--)
    {
      writeData(pgm_read_byte(addr++));
    }

    if (ms)
    {
      ms = pgm_read_byte(addr++);
      if (ms == 255)
      {
        ms = 500;
      }
      delay(ms);
    }
  }
}

// ----------------------------------------------------------
void DrvIli9341::setRotation(uint8_t m)
{
  this->rotation = m & 3;
  switch (this->rotation)
  {
  case 0:
    this->_width = DRV_ILI9341_TFTWIDTH;
    this->_height = DRV_ILI9341_TFTHEIGHT;
    break;
  case 1:
    this->_width = DRV_ILI9341_TFTHEIGHT;
    this->_height = DRV_ILI9341_TFTWIDTH;
    break;
  case 2:
    this->_width = DRV_ILI9341_TFTWIDTH;
    this->_height = DRV_ILI9341_TFTHEIGHT;
    break;
  case 3:
    this->_width = DRV_ILI9341_TFTHEIGHT;
    this->_height = DRV_ILI9341_TFTWIDTH;
    break;
  }
}

// ----------------------------------------------------------
void DrvIli9341::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
  // optimized version
  CS_ACTIVE;
  SPI_START;

  DC_COMMAND;
  writeSPI(ILI9341_CASET);
  DC_DATA;
  writeMulti(xs, 1);
  writeMulti(xe, 1);

  DC_COMMAND;
  writeSPI(ILI9341_PASET);
  DC_DATA;
  writeMulti(ys, 1);
  writeMulti(ye, 1);

  DC_COMMAND;
  writeSPI(ILI9341_RAMWR);

  DC_DATA;
  // no CS_IDLE + SPI_END, DC_DATA to save memory
}

// ----------------------------------------------------------
void DrvIli9341::pushColor(uint16_t color)
{
  SPI_START;
  // DC_DATA;
  CS_ACTIVE;

  writeSPI(color >> 8);
  writeSPI(color);

  CS_IDLE;
  SPI_END;
}

// ----------------------------------------------------------
#ifdef DRV_ILI9341_RAM_FOR_SPEED
void DrvIli9341::handleRotation(int16_t *x, int16_t *y)
{
  int16_t temp;

  switch (this->rotation)
  {
  case 0:
    break;
  case 1:
    temp = *y;
    *y = *x;
    *x = DRV_ILI9341_TFTWIDTH - temp;
    break;
  case 2:
    *x = DRV_ILI9341_TFTWIDTH - *x;
    *y = DRV_ILI9341_TFTHEIGHT - *y;
    break;
  case 3:
    temp = *x;
    *x = *y;
    *y = DRV_ILI9341_TFTHEIGHT - temp;
  }
}

void DrvIli9341::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  this->handleRotation(&x, &y);

  if (x < 0 || x >= DRV_ILI9341_TFTWIDTH || y < 0 || y >= DRV_ILI9341_TFTHEIGHT)
    return;

  this->pixels[y * DRV_ILI9341_TFTWIDTH + x] = color;
}
#else
void DrvIli9341::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  if (x < 0 || x >= this->_width || y < 0 || y >= this->_height)
    return;
  setAddrWindow(x, y, x + 1, y + 1);

  // writeMulti(color,1);
  writeSPI(color >> 8);
  writeSPI(color);

  CS_IDLE;
  SPI_END;
}
#endif

// ----------------------------------------------------------
void DrvIli9341::drawBuffer(uint16_t *buffer)
{
#ifdef DRV_ILI9341_RAM_FOR_SPEED
  setAddrWindow(0, 0, DRV_ILI9341_TFTWIDTH - 1, DRV_ILI9341_TFTHEIGHT - 1);
  SPI.transfer((uint8_t *)buffer, 2 * DRV_ILI9341_TFTHEIGHT * DRV_ILI9341_TFTWIDTH);
#endif
}