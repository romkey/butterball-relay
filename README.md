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

Butterball works reliably. It is quite dependent on a stable power supply.

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

## Software

Butterball's firmware uses the Arduino SDK for ESP32 and is organized to build using [PlatformIO](https://platformio.org/), which runs under Linux, MacOS and Windows.

The software provides a uniform, abstract interface to each device using the `Sensor` class. The `Sensor` class provides some housekeeping functions to help the software poll the devices at the appropriate intervals.

# License

Software and documentation are licensed under the [MIT license](https://romkey.mit-license.org/).

Hardware designs are licensed under [Creative Commons Attribution Share-Alike license](https://creativecommons.org/licenses/by-sa/4.0). 
