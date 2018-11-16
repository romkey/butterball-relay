# The Butterball

Butterball (not associated in any way with the turkey company) is an open-source hardware project based on [Furball](https://github.com/HomeBusProjects/furball)/[Dustball](https://github.com/romkey/dustball-client), which are open-source hardware projects intended to help with the research and development of HomeBus, an IoT auto-provisioning framework.

HomeBus is nowhere near ready to go and we want to use Butterball now, so Butterball will start out reporting via a REST API to the Butterball server. It can also be configured to report to an MQTT server.

The name 'Furball' is an homage to Dave Mills' ["Fuzzball"](https://en.wikipedia.org/wiki/Fuzzball_router), one of the first routers on the nascent Internet.

The Butterball hardware performs high temperature monitoring. It uses a type K thermocouple, suitable for temperatures from 0 to several hundred degrees C (the exact limits depend on the probe you use - the MAX6675 can measure to 1024°C). Butterball is intended for temperature monitoring applications like:
- water temperature in a sous vide bath
- temperature inside a grill or oven
- coffee roasting
- oil temperature inside a fryer

*Please be extremely careful working in high temperature environments. Use a thermocouple properly rated for the environment with a long enough cable that's tolerant of the temperatures and conditions it may be exposed to. Keep the electronics distant from the high temperature environment. I cannot advise you on the suitability of any equipment for any particular environment.*

## Current Status

Butterball works reliably when powered by a stable, sufficient power supply.

## Hardware

Butterball, Furball and Dustball are based on the ESP32 processor. ESP32 is more capable than its predecessor, the ESP8266 while remaining inexpensive. The ESP32 supports both 802.11b/g/n Wifi and Bluetooth 4.2/BLE. It also has hardware acceleration for encryption. It includes 4 SPI controllers, 2 I2C controllers, 3 UARTs, up to 18 channels of ADC and two 8 bit DACs. 

Butterball uses a MAX6675 temperature sensor, an optional BME280 temperature/humidity/pressure sensor and an optional monochrome OLED display. It's powered via USB.

### Total Cost

If bought through AliExpress, parts cost should run roughly:
- $6 - ESP32 LOLIN32
- $2.57 - BME280
- $1.55 - MAX6675
- $1.59 - SSD1306-based I2C OLED display
- $1.41 - K-type thermocouple temperature probe
- $1 - random resistors, capacitors

Total of roughly $15 in parts before the circuit board, wire, solder.

## Building The Hardware

I'm still working on a schematic, but the hardware is very simple to build.

Connect 3.3V power and ground from the ESP32 to each of the MAX6675, BME280, OLED display.

Connect the I2C SDA and SDC lines from the ESP32 to the BME280 and OLED display. Also connect SDA and SDC each via a 470K resistor to 3.3V.  SDA and SDC are run open-collector and need pull up resistors.

Install a large (100µF) capacitor between 3.3V and ground, as phyiscally close to the ESP32 as possible. Also install a small (100pF) capacitor between 3.3V and ground as physically close to each of the MAX6675, BME280 and OLED display. These will help stabilize power to each device.

Try wiring up the I2C devices each one at a time and testing each time to make sure they work. If you run into problems, try using the ESP32 diagnostic tool to do an I2C scan - it's possible your I2C board might be using a different address than the software defaults to.

### Wemos D1 Mini Wiring Guide

If you're using a different ESP8266 module, change the pin numbers as needed.

You can simply omit the BME280 and/or OLED display if you want. If you don't use either you won't need the pullup resistors. You only need the small 100pF capacitor if you're using the board it's wired for.

Wemos pin 8 (3.3V) - MAX6675 VCC, BME280 VCC, OLED display VCC, 2 pullup resistors, 100µF capacitor (positive lead)
Wemos pin 15 (GND) - MAX6675 GND, BME280 GND, OLED display GND, 100µF capacitor (negative lead)
Wemos pin 19 (GPIO4, SDA) - BME280 SDA, OLED display SDA, pull resistor 
Wemos pin 20 (GPIO5, SCL) - BME280 SCL, OLED display SCL, pull resistor
Wemos pin 5 (GPIO14, SCK) - MAX6675  
Wemos pin 6 (GPIO12, MISO) - MAX6675  
Wemos pin 7 (GPIO13, MOSI) - MAX6675  
BME280 - wire a 100pF capacitor across VCC and GND as close to the breakout board as possible 
OLED display - wire a 100pF capacitor across VCC and GND as close to the breakout board as possible 
so -miso - 6 - gpio 12 
cs- mosi - 7 - gpio 13
sck - sck - 5 - gpio 14

## Software

Butterball's firmware uses the Arduino SDK for ESP32 and is organized to build using [PlatformIO](https://platformio.org/), which runs under Linux, MacOS and Windows.

The software provides a uniform, abstract interface to each device using the `Sensor` class. The `Sensor` class provides some housekeeping functions to help the software poll the devices at the appropriate intervals.

To build the software, first install [Platformio](https://platformio.org).

Then

1. `git clone https://github.com/romkey/butterball-client`
2. `cd butterball-client`
3.  copy `config.h-example` to `config.h` and edit it to set it up properly
3. `platformio run -e lolin32`

If you're building for a Wemos D1 Mini that last command will be `platformio run -e d1_mini`

Platformio will automatically download the needed toolchain, the Arduino framework and the necessary libraries.

When you're ready to flash the firmware on your board  plug it into your computer via USB and run `platformio run -e lolin32 -t upload`

Run `platformio device monitor` to see the serial output from the device


# License

Software and documentation are licensed under the [MIT license](https://romkey.mit-license.org/).

Hardware designs are licensed under [Creative Commons Attribution Share-Alike license](https://creativecommons.org/licenses/by-sa/4.0). 
