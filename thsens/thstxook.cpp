// 2015-02-03 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

#include "thsconfig.h"
#include "thstxook.h"

THSTxOOK::THSTxOOK(int txPinIn, int syncHiIn, int syncLoIn,
                   int zeroHiIn, int zeroLoIn, int oneHiIn, int oneLoIn)
 : txPin(txPinIn), syncHi(syncHiIn), syncLo(syncLoIn),
   zeroHi(zeroHiIn), zeroLo(zeroLoIn), oneHi(oneHiIn), oneLo(oneLoIn) {
}

void THSTxOOK::init() {
  // configure transmitter pin as output
  pinMode(txPin, OUTPUT); 
  digitalWrite(txPin, LOW);
#ifdef TX_INDICATOR_LED_PIN
  pinMode(TX_INDICATOR_LED_PIN, OUTPUT); 
  digitalWrite(TX_INDICATOR_LED_PIN, LOW);
#endif
}

void THSTxOOK::transmitBit(int numHighPulses, int numLowPulses) {
  digitalWrite(txPin, HIGH);
  delayMicroseconds(numHighPulses);
  digitalWrite(txPin, LOW);
  delayMicroseconds(numLowPulses);
}

void THSTxOOK::transmitMessage(const char* message) {
#ifdef TX_INDICATOR_LED_PIN
  digitalWrite(TX_INDICATOR_LED_PIN, HIGH);
#endif

  for (int i = 0; i < NUM_TX_REPETITIONS; i++) {
    unsigned int pos = 0;
    transmitBit(syncHi,syncLo);
    while (message[pos] != '\0') {
      switch(message[pos]) {
      case '0':
        transmitBit(zeroHi, zeroLo);
        break;
      case '1':
        transmitBit(oneHi, oneLo);
        break;
      }
      pos++;
    }
    switch(message[pos-1]) {
    case '0':
      transmitBit(zeroHi, zeroLo);
      break;
    case '1':
      transmitBit(oneHi, oneLo);
      break;
    }
    transmitBit(syncLo, syncLo);
  }

#ifdef TX_INDICATOR_LED_PIN
  digitalWrite(TX_INDICATOR_LED_PIN, LOW);
#endif
}

