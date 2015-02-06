// 2015-02-03 <mhprog@gmx.net> http://opensource.org/licenses/mit-license.php

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include "thsconfig.h"
#include "thsutils.h"
#include "thsprotocolect.h"

#define SYNC_HI  200
#define SYNC_LO  8050
#define ZERO_HI 200
#define ZERO_LO 1010
#define ONE_HI  200
#define ONE_LO  2020

THSProtocolECT::THSProtocolECT()
  : THSProtocol(SYNC_HI, SYNC_LO, ZERO_HI, ZERO_LO, ONE_HI, ONE_LO) {
    message.reserve(64);
}

/*
 I  Device Id
 C  Channel
 B  Battery low
 TT Trend
 F  Force send
 t  temperature
 h  humidity
 c  checksum
 */

String THSProtocolECT::encode() {
  message = "";
  
  // ID Part 1
  message = THSUtils::int2bin(deviceId, 4);

  // channel
  message += THSUtils::int2bin(channel, 2);

  // ID Part 2
  message += THSUtils::int2bin(deviceId >> 4, 2);

  // Low battery warning bit (1 = low batt)
  message += batteryOk ? "0" : "1";

  // trend ( 0 = cont., 1 = rising, 2 = falling )
  message += THSUtils::int2bin(0, 2);

  // forced send bit (1 = forced)
  message += manualSend ? "1" : "0";

  // unknown
  message += THSUtils::int2bin(0, 5);

  // humidity
  int h = int(humidity);
  message += THSUtils::int2bin(h, 7);

  // temperature
  int t;
  if (temperature < 0) {
    t = (int((-temperature) * 10.0) & 0xeff) + 0x800;
  } 
  else {
    t = int(temperature * 10.0) & 0xeff;
  }
  message += THSUtils::int2bin(t, 12);

  return message;
}
