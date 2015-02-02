# thsens
Wireless temperature and humidity sensor

thsens is a Arduino sketch which measures temperature and humidity
from a DHT22 sensor and transmits it via a cheap FS1000A 433MHz.
It is primarily used as remote environmental sensor in a home automation setup.

The protocol used will be detected e.g. by the fhemduino project http://www.fhemwiki.de/wiki/FHEMduino
(as EuroChron/Tchibo device).

Particular emphasis was placed on low power consumption, because the device should run from AA batteries
at least a year. With my setup I measured 35µA during idle (no transmission) period and 15mA during transmission.
Assuming a transmission duration of 1 second and idle period of 15 minutes we get a mean current of ~50µA.
One set of batteries should therefore last a year or more.

# Requirements
1. Arduino IDE (V1.0.5 used, did not test other versions)
2. LowPower library (http://www.rocketscream.com/blog/2011/07/04/lightweight-low-power-arduino-library)
3. DHT library (https://github.com/adafruit/DHT-sensor-library)
4. TrueRandom library (https://code.google.com/p/tinkerit/wiki/TrueRandom)

# Usage
At first start a random device and channel id will be generated and stored in EEPROM.
The default transmission interval is 15 minutes. Thus, every 15 minutes temperature and humidity will be
measured and transmitted. Additionally the battery voltage will be determined, and, if below a certain level,
a battery low flag will be set in the transmitted messages.

## Configuration options
### Compile time configurations
1. THS_DEBUG	enable debug output and command line interface via default arduino serial port
2. DHT_PIN	pin number of the DHT sensor data pin
3. DHTTYPE	type of the DHT sensor (the library supports several type, but only DHT22 has been tested by me)
4. TRANSMITTER_PIN	pin number where the tx module data pin is connected to
5. TX_INDICATOR_LED_PIN	define this  to indicate transmission period via led (connected to this pin)
6. MIN_VCC	minimum battery voltage in mV (for battery low flag in transmission messages)

### Runtime settings
These setting will be stored in EEPROM and are only changeable via serial command line interface (THS_DEBUG set)

1. device id	unique number (0...63)
2. channel	number (0...3) device id + channel form the complete device address
3. tx interval  in minutes (measure and send every n minutes, valid 1...255)

### Commands (via serial interface)
The serial interface uses 57600baud. Each command has to be terminated by a newline.

1. h	short help
2. V	version information
3. c	set options (format: cxx,y,z  (xx two-digit device id, y one-digit channel, z tx delay in minutes)
4. E	immediately measure and transmit
5. S	measure temperature and humidity and print it via serial
6. i	show device information (battery voltage, CPU temperature...)

# My setup
I use a Arduino Pro Mini 5V (328). It will be powered directly by 3 AA batteries (connected to VCC, not Vin).

To reduce power consumption I removed the power led resistor and dc converter.
The schematics can be found in the doc directory.

