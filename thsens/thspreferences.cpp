
// 2015-01-28 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#include <EEPROM.h>
#include <TrueRandom.h>
#include "thspreferences.h"

// load preference values from eeprom
// init eeprom and prefs values if eeprom has not the correct format
void THSPreferences::load()
{
  // check magic bytes in eeprom
  if (EEPROM.read(EE_PREF_ADDR_MAGIC1) != EE_PREF_MAGIC1 ||
      EEPROM.read(EE_PREF_ADDR_MAGIC2) != EE_PREF_MAGIC2) {
    // magic bytes not found, assuming EEPROM is not initialized
    // write eeprom magic bytes
    EEPROM.write(EE_PREF_ADDR_MAGIC1, EE_PREF_MAGIC1);
    EEPROM.write(EE_PREF_ADDR_MAGIC2, EE_PREF_MAGIC2);
    // initialice device id and channel with random numbers
    deviceId = TrueRandom.random(64);
    channel = TrueRandom.random(4);
    // initialize tx interval to 15 (minutes)
    txInterval = 15;
    save();
  } else {
    deviceId = EEPROM.read(EE_PREF_ADDR_DEVICE_ID);
    channel = EEPROM.read(EE_PREF_ADDR_CHANNEL);
    txInterval = EEPROM.read(EE_PREF_ADDR_TX_INTERVAL);
  }

}

// save preference values to eeprom
void THSPreferences::save()
{
  uint8_t eeValue;
  
  eeValue = EEPROM.read(EE_PREF_ADDR_DEVICE_ID);
  if (eeValue != deviceId) {
    EEPROM.write(EE_PREF_ADDR_DEVICE_ID, deviceId);
  }

  eeValue = EEPROM.read(EE_PREF_ADDR_CHANNEL);
  if (eeValue != channel) {
    EEPROM.write(EE_PREF_ADDR_CHANNEL, channel);
  }

  eeValue = EEPROM.read(EE_PREF_ADDR_TX_INTERVAL);
  if (eeValue != deviceId) {
    EEPROM.write(EE_PREF_ADDR_TX_INTERVAL, txInterval);
  }
}

