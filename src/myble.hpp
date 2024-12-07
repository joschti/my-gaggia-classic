#ifndef _MYBLE_HPP_
#define _MYBLE_HPP_

#ifdef BLE_ENABLE

#include "Arduino.h"
#include <ArduinoBLE.h>

#include "mytypes.h"
#include "mytask.hpp"

class MyBle : public MyTask
{
public:
    MyBle(uint32_t msUpdateRate);
    void init();
    void taskHandler(interface_ble_t payload);

private:
    BLEService envSenseService;
    BLECharacteristic tempCharacteristic;
    BLECharacteristic configCharacteristic;
    uint8_t cfgCount;
    //   BLECharacteristic humidityCharacteristic;
    //   BLECharacteristic ledControlCharacteristic;

    // BLEDevice central;
};

#endif

#endif
