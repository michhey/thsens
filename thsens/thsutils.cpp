// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>
#include "thsutils.h"

// read battery voltage
// ATTENTION: this AVR specific and only supported by some models.
// See e.g. at https://code.google.com/p/tinkerit/wiki/SecretVoltmeter for further information
long THSUtils::getBatteryVoltage()
{
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
//  result = ADCL;
//  result |= ADCH<<8;
  result = ADCW;
  result = 1126400L / result;
  return result;
}

// read AVR CPU temperature
// ATTENTION: this AVR specific and only supported by some models.
// See e.g. at https://code.google.com/p/tinkerit/wiki/SecretThermometer for further information
// temperature is in degrees Celsius
float THSUtils::getCPUTemperature()
{
  float result;
  unsigned int wADC;
  // Read temperature sensor against 1.1V reference
  ADMUX = _BV(REFS1) | _BV(REFS0) | _BV(MUX3);
  delay(20); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  wADC = ADCW;
  result = (wADC - 324.31) / 1.22;
  return result;
}

// convert numbits bits of an int value into a string with binary numbers
// e.g. value 5 with 4 bits -> "0101"
String THSUtils::int2bin(int value, int numbits) {
  String message = "";
  int mask = 1 << (numbits - 1);
  for (int i = 0; i < numbits; i++) {
    if (value & mask) {
      message += "1";
    } 
    else {
      message += "0";
    }
    mask >>= 1;
  }

  return message;
}

// convert a string with binary numbers into int
// e.g. "0101" -> 5
int THSUtils::bin2int(String message) {
  int value = 0;

  for (unsigned int pos = 0; pos < message.length(); pos++) {
    value <<= 1;
    if (message.charAt(pos) == '1') {
      value |= 1;
    }
  }
  return value;
}

