#ifndef _TYPES_H_
#define _TYPES_H_

#include "Arduino.h"

//-----------------------------------------------------------------
/* Type definitions */
typedef struct
{
    float temp;
} payload_ble_t;

typedef struct
{
    bool temp;
} update_ble_t;

typedef struct
{
    update_ble_t update;
    payload_ble_t payload;
} interface_ble_t;

#endif
