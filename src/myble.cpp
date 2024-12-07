#ifdef BLE_ENABLE

#include "myble.hpp"
#include "config.hpp"

MyBle::MyBle(uint32_t msUpdateRate)
    : MyTask(msUpdateRate),
      envSenseService("181A"),
      tempCharacteristic("2A6E", BLERead | BLENotify, 4, true),
      configCharacteristic("2A6F", BLEWrite, 4 * 4, true)
//   humidityCharacteristic("2A6F", BLERead | BLENotify, 4, true),
//   ledControlCharacteristic("20212134-edf6-45d7-b5a5-6c0655651363", BLERead | BLEWrite, 3, true)
{
}

void MyBle::init()
{
    if (!BLE.begin())
    {
        Serial.println("Failed to initialize Bluetooth LE!");
        while (1)
            ;
    }

    // set advertised local name and service UUID:
    BLE.setLocalName("Gaggia Classic Extension");
    BLE.setAdvertisedService(envSenseService);
    // BLE.setConnectionInterval

    // add the characteristic to the service
    this->envSenseService.addCharacteristic(this->tempCharacteristic);
    this->envSenseService.addCharacteristic(this->configCharacteristic);
    // this->envSenseService.addCharacteristic(this->humidityCharacteristic);
    // this->envSenseService.addCharacteristic(this->ledControlCharacteristic);

    // add service
    BLE.addService(this->envSenseService);

    // set the initial value for the characteristic:
    this->tempCharacteristic.writeValue((uint32_t)0, false);
    // this->humidityCharacteristic.writeValue((uint32_t)0, false);
    // uint8_t zeros[3] = {0};
    // this->ledControlCharacteristic.writeValue(&zeros[0], 3, false);

    // dummy write this variable
    // this->central = BLE.central();

    this->cfgCount = 0;

    // start advertising
    BLE.advertise();
}

void MyBle::taskHandler(interface_ble_t interface)
{
    // if a central is connected to peripheral
    BLEDevice central = BLE.central();
    if (central.connected())
    {
        // turn on the LED to indicate the connection
        digitalWrite(LED_BUILTIN, HIGH);

        if (this->timeHandler())
        {
            // send temperature value
            this->tempCharacteristic.writeValue(*(uint32_t *)&interface.payload.temp, false);
        }

        // read all the time, if value is updated
        if (configCharacteristic.written())
        {
            switch (this->cfgCount)
            {
            case 0:
                this->configCharacteristic.readValue((uint32_t &)config::targetTemp);
                break;
            case 1:
                this->configCharacteristic.readValue((uint32_t &)config::ctrlHeaterProp);
                break;
            case 2:
                this->configCharacteristic.readValue((uint32_t &)config::ctrlHeaterInt);
                break;
            case 3:
                this->configCharacteristic.readValue((uint32_t &)config::ctrlHeaterDiff);
                break;
            }
            // sequentially receive the configs (specific order of transmission is required)
            this->cfgCount++;
            this->cfgCount = this->cfgCount > 3 ? 0 : this->cfgCount;

            char buffer[70];
            sprintf(buffer, "%f,%f,%f,%f\n", config::targetTemp, config::ctrlHeaterProp, config::ctrlHeaterInt, config::ctrlHeaterDiff);
            Serial.print(buffer);
        }
    }
    else
    {
        // when the central disconnects, turn off the LED
        digitalWrite(LED_BUILTIN, LOW);
    }
}

#endif