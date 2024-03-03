# FROM ubuntu:20.04

# ENV DEBIAN_FRONTEND=noninteractive

# RUN apt-get update \
#     && apt-get install -y curl udev usbutils

# RUN curl -LJO https://downloads.arduino.cc/arduino-cli/arduino-cli_latest_Linux_64bit.tar.gz \
#     && tar xvzf arduino-cli_latest_Linux_64bit.tar.gz \
#     && rm arduino-cli_latest_Linux_64bit.tar.gz \
#     && mv arduino-cli /usr/bin

# RUN arduino-cli core install arduino:mbed_nano \
#     && arduino-cli lib install ArduinoBLE

ARG VARIANT=20-bullseye
FROM mcr.microsoft.com/vscode/devcontainers/typescript-node:0-${VARIANT}

RUN <<EOF 
  apt-get update 
  export DEBIAN_FRONTEND=noninteractive 
  apt-get -y install --no-install-recommends \
    clang \
    curl \
    udev \
    usbutils 
  
  curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
  service udev restart 
  usermod -a -G dialout node
  usermod -a -G plugdev node 

EOF

# todo: need to copy ssh files, and change access writes with chmod 600 <file>