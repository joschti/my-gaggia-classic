#include "drv_m5sonic.h"

/*! @brief Initialize the Sonic. */
void DrvM5Sonic::begin(uint8_t trig, uint8_t echo)
{
    _trig = trig
        _echo = echo;
    pinMode(_trig,
            OUTPUT); // Sets the trigPin as an Output. 将 TrigPin 设置为输出
    pinMode(_echo,
            INPUT); // Sets the echoPin as an Input. 将 echoPin 设置为输入
}

float DrvM5Sonic::getDuration()
{
    digitalWrite(_trig, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds.

    digitalWrite(_trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trig, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    // todo jvo: pulseIn might be dangerous to use, hovering over it reveals a few more NRF specific stuff...
    float duration = pulseIn(_echo, HIGH);

    return duration;
}

/*! @brief Get distance data. */
float DrvM5Sonic::getDistance()
{
    // Calculating the distance
    // todo jvo: use SPEED_OF_SOUND_AIR
    float Distance = getDuration() * 0.34 / 2;
    if (Distance > 4500.00)
    {
        return 4500.00;
    }
    else
    {
        return Distance;
    }
}