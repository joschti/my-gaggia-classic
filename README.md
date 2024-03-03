# My Gaggia Classic Extension

__TOC__

## Development environment 

### Setting up

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

### Open

1. Open WSL and open VS code via
    - `code path-to-project-folder`
2. Press `CTRL`+`SHIFT`+`P` and choose `Dev Containers: Reopen in Container`

### PIO tasks

Most actions are done via tasks or via command line instruction e.g. compilation, upload build to hardware, etc. Platform IO provides and deals with these things.

## Firmware

### Tips and tricks

#### Valuable paths
- NRF52 SDK
    - `/home/node/.platformio/packages/framework-arduino-mbed/cores/arduino/mbed/targets/TARGET_NORDIC/TARGET_NRF5x/TARGET_SDK_15_0/modules/nrfx`

### Workarounds

#### Remove from /home/node/.platformio/packages/framework-arduino-mbed/variants/ARDUINO_NANO33BLE/includes.txt
Need to remove this line because it includes the `sdk_config.h` from the Arduino package. However, some things need to be enabled that are not enabled by default like a few timers.
```
-iwithprefixbefore/mbed/targets/TARGET_NORDIC/TARGET_NRF5x/TARGET_NRF52/TARGET_MCU_NRF52840/config
```

Note 1: TIMER0 and 2 are not enabled. These seem to be used by something else (maybe from Arduino or mbed).

Note 2: TIMER1's `NRF_TIMER_CC_CHANNEL0` seems to be broken and often has outages. Therefore, it should remain unused. A look at the errata sheet could give additional intel.

#### Include NRF driver source files
In the Arduino NRF SDK package, the NRF driver source files are not present. Therefore, these files are copied from the SDK to the `src/drv` folder.

## Android app with MIT App Inventor 2

The project can be edited and compiled online: https://ai2.appinventor.mit.edu. 

## Data analysis

### Data logging via serial output

Use `pio device monitor -f default -f log2file` to log the serial output to a file. For more information about the format, check out `serialout.hpp` or [pio device monitor filter options](https://docs.platformio.org/en/latest/core/userguide/device/cmd_monitor.html#cmd-device-monitor-filters).

### Measurements on Gaggia Classic

Heating (invalid due to poor thermal connection between sensor and boiler):
- Stops: 83°C
- Starts: 69°C
- Stops (again): 75°C?

## TODO
- Devcontainer clean up
- Update measurements
- Block diagrams (FW, Gaggia)
- Draft schematic of Gaggia