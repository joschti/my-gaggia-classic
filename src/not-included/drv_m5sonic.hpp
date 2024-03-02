// (https://docs.m5stack.com/en/unit/sonic.io)

#ifndef _DRV_M5_SONIC_H_
#define _DRV_M5_SONIC_H_

#include "Arduino.h"
#include "pins_arduino.h"

#define SPEED_OF_SOUND_AIR 340

class DrvM5Sonic
{
private:
    uint8_t _trig;
    uint8_t _echo;

public:
    void begin(uint8_t trig = 26, uint8_t echo = 36);
    float getDuration();
    float getDistance();
};

#endif