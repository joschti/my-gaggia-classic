# My Gaggia Classic Extension

## Setting up

1. Open WSL and run
    - `sudo service docker start`
2. Open Windows PowerShell (with administrator rights) and run
    - `usbipd list`
    - Identify your device
    - `usbipd attach --wsl --busid <BUS-ID> --auto-attach`
    - Not so stable (see error message below)
```
WSL usbip: error: Attach Request for 2-1 failed - Device busy (exported)
usbipd: warning: The device appears to be used by Windows; stop the software using the device, or bind the device using the '--force' option.
```

## Tips and tricks

### Valuable paths
- NRF52 SDK
    - `/home/node/.platformio/packages/framework-arduino-mbed/cores/arduino/mbed/targets/TARGET_NORDIC/TARGET_NRF5x/TARGET_SDK_15_0/modules/nrfx`

## Workarounds

### Remove from /home/node/.platformio/packages/framework-arduino-mbed/variants/ARDUINO_NANO33BLE/includes.txt
Need to remove this line because it includes the `sdk_config.h` from the Arduino package. However, some things need to be enabled that are not enabled by default like a few timers.
```
-iwithprefixbefore/mbed/targets/TARGET_NORDIC/TARGET_NRF5x/TARGET_NRF52/TARGET_MCU_NRF52840/config
```

Note 1: TIMER0 and 2 are not enabled. These seem to be used by something else (maybe from Arduino or mbed).

Note 2: TIMER1's `NRF_TIMER_CC_CHANNEL0` seems to be broken and often has outages. Therefore, it should remain unused.

### Include NRF driver source files
In the Arduino NRF SDK package, the NRF driver source files are not present. Therefore, these files are copied from the SDK to the `src/drv` folder.

## Measurements on Gaggia Classic

Heating:
- Stops: 83°C
- Starts: 69°C
- Stops (again): 75°C?

## TODO
- Devcontainer clean up
- Update measurements w