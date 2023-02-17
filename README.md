# VT2040-serial
RP2040 PIO serial port, created for the VT2040 terminal. Intended for communicating with an ESP8266 running [Retro WiFi Modem](https://github.com/mecparts/RetroWiFiModem), [MicroPython](https://docs.micropython.org/en/latest/esp8266/tutorial/intro.html), [CP/M](https://github.com/mengstr/cpm8266), or other firmware that communicates via serial.

## Features
* Can use any GPIO pins for TX and RX
* 115200 8N1, compatible with most ESP8266 firmwares
* Ignores ESP8266 boot messages
* 4kB receive buffer (configurable)
